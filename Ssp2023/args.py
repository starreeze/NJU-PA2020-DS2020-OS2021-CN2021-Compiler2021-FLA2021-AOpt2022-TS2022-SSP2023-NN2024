# -*- coding: utf-8 -*-
# @Date    : 2023-12-15 16:03:07
# @Author  : Starreeze

llama_path = "/home/nfs02/model/llama2/hf/Llama-2-7b-chat-hf"
llama_instruction_placeholder = "$$$"
llama_sys_prompt = (
    "<<SYS>>\nYou are a helpful, respectful and honest assistant. "
    "Strictly follow the instruction and always answer as helpfully as possible.\n"
    f"<</SYS>>\n\n</s> [INST] {llama_instruction_placeholder} [/INST] "
)

weather_api = "https://api.openweathermap.org/data/2.5/{type}?lat={lat}&lon={lon}&units=metric&appid=efb89fb822a17cfa3a86075f8c05c612"
api_type_mapping = {"current": "weather", "forecast": "forecast"}
location_api = "http://api.openweathermap.org/geo/1.0/direct?q={}&limit=1&appid=efb89fb822a17cfa3a86075f8c05c612"

default_city = "Nanjing"
forecast_weather_len = 10

logging_level = "DEBUG"
logging_format = "%(asctime)s - %(levelname)s - %(message)s"

server_ip = "0.0.0.0"
server_port = 60080
proxy = "http://114.212.105.13:7890"
