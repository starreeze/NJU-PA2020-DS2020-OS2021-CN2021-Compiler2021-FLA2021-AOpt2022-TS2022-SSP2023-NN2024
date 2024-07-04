# -*- coding: utf-8 -*-
# @Date    : 2023-12-15 17:29:00
# @Author  : Starreeze

from __future__ import annotations
import os, sys, logging, requests, json
from typing import Callable
from datetime import datetime

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))
import args
from nlu.llama import LlamaChat

logging.basicConfig(level=args.logging_level, format=args.logging_format)
proxy = {"http": args.proxy, "https": args.proxy}


def get_json(url: str):
    return json.loads(requests.get(url, proxies=proxy).text)


def ask_llm(
    prompt: str,
    query: str,
    max_new_tokens: int,
    condition: Callable[[str], bool] = lambda x: True,
    post_process: Callable[[str], str] = lambda x: x,
    retries=3,
    default="",
) -> str:
    chat = LlamaChat(prompt, max_new_tokens)
    for i in range(retries):
        response = post_process(chat.infer(query))
        if condition(response):
            return response
        logging.warning(
            "Failed to get a valid response from the chatbot. "
            f"Prompt: {prompt} Query: {query} Response: {response}. Retrying ({i+1}/{retries})..."  # type: ignore
        )
    return default


def get_current_or_forecast(query: str) -> str:
    prompt = (
        "Now you will be provided with a user's query related with weather conditions, "
        "and you need to determine if the user is "
        "seeking information about the current weather conditions (current) "
        "or if they are looking for a weather forecast (forecast). "
        "Classify the intent and provide the appropriate label [current/forecast]. "
        "Note that your reply should be either 'current' or 'forecast'.\n"
    )
    query = "(Query) {} (Label) ".format(query)

    def current_or_forcast(x: str) -> str:
        x = x.lower()
        if "current" in x and "forecast" not in x:
            return "current"
        if "forecast" in x and "current" not in x:
            return "forecast"
        return ""

    response = ask_llm(
        prompt, query, 50, lambda x: x == "current" or x == "forecast", current_or_forcast, default="current"
    )
    logging.info(f"This is a {response} query.")
    return response


def get_location(query: str) -> tuple[float, float]:
    prompt = (
        "Now you will be provided with a user's query related with weather conditions, "
        "and you need to determine which city the user is seeking information about. "
        "Provide the city name according the query. If the query does not mention the city, "
        "reply 'None' instead. "
        "Note that your reply should **only contain city name or be 'None'**, "
        "without explanation, punctuation or any other symbols.\n"
        f"Example_1: (Query) Is the sky clear now in {args.default_city}? (City name) {args.default_city}\n"
        "Example_2: (Query) Will tomorrow be colder? (City name) None\n"
    )
    query = "(Query) {} (City name) ".format(query)
    city = ask_llm(prompt, query, 10)
    logging.info(f"This is a query for weather in {city}.")
    city = (city[p:] if (p := city.find(":")) != -1 else city).strip(":;,.'\"")
    city = args.default_city if city.lower() == "none" else city
    response = get_json(args.location_api.format(city))
    location = response[0]["lat"], response[0]["lon"]
    logging.info(f"The location is: {location}.")
    return location


def retrieve_info(query: str, info: str) -> str:
    prompt = (
        "Now you will be provided with a user's query related with weather conditions "
        "and some related information from openweather, and you need to generate "
        "truthful, fluent and informative response to the query according to the information. "
        "Note that when you say time, you should emphasize that the time is UTC. "
        "Your reply must be based on the information provided, "
        "and you should not invent information that is not mentioned.\n"
    )
    query = f"(Info) {info} (Query) {query} (Response) "
    response = ask_llm(prompt, query, 500)
    logging.info("Got final response.")
    logging.debug(response)
    return response


def timestamp_to_str(timestamp: int) -> str:
    utc_dt = datetime.utcfromtimestamp(timestamp)
    return utc_dt.time().strftime("%H:%M")


def api_response_to_weather_info(response: dict):
    targets = {
        "weather": ["weather", 0, "description"],
        "temperature_centigrade": ["main", "temp"],
        "feels_like_centigrade": ["main", "feels_like"],
        "temperature_min_in_hour": ["main", "temp_min"],
        "temperature_max_in_hour": ["main", "temp_max"],
        "pressure": ["main", "pressure"],
        "humidity": ["main", "humidity"],
        "visibility_meter": ["visibility"],
        "wind_speed": ["wind", "speed"],
        "wind_direction_degree": ["wind", "deg"],
        "wind_gust": ["wind", "gust"],
        "clouds": ["clouds", "all"],
    }
    results = {}
    for k, v in targets.items():
        try:
            r = response
            for i in v:
                r = r[i]
            results[k] = r
        except KeyError:
            pass
    return results


def answer(query: str) -> str:
    logging.info(f"received query: {query}")
    type = get_current_or_forecast(query)  # to determine which api to use
    lat, lon = get_location(query)
    url = args.weather_api.format(type=args.api_type_mapping[type], lat=lat, lon=lon)
    response = get_json(url)

    if type == "current":
        info = api_response_to_weather_info(response) | {
            "sunrise_time_UTC": timestamp_to_str(response["sys"]["sunrise"]),
            "sunset_time_UTC": timestamp_to_str(response["sys"]["sunset"]),
        }
    else:
        response = response["list"][:: len(response["list"]) // args.forecast_weather_len]
        info = {
            "current_time_UTC": datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"),
            "forecast": [{"time_UTC": r["dt_txt"]} | api_response_to_weather_info(r) for r in response],
        }
    return retrieve_info(query, json.dumps(info))


if __name__ == "__main__":
    while True:
        print(answer(input("> ")))
