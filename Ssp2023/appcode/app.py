from flask import Flask, request, render_template, jsonify, send_file
import os
os.environ['http_proxy'] = 'http://114.212.105.13:7890'
os.environ['https_proxy'] = 'http://114.212.105.13:7890'
app = Flask(__name__)
import requests
import re
import paddle
from paddlespeech.cli.asr import ASRExecutor
asr_executor = ASRExecutor()

from paddlespeech.cli.tts import TTSExecutor
tts_executor = TTSExecutor()
import argparse
from google.cloud.speech_v2 import SpeechClient
from google.cloud.speech_v2.types import cloud_speech

from google.cloud import texttospeech
client_tts = texttospeech.TextToSpeechClient()

llmurl = "http://210.28.134.152:60080"

def clean_string(s):
    # This pattern will match any characters that are NOT English letters, numbers, punctuation, or the degree symbol
    pattern = '[^a-zA-Z0-9°\s\.,!?;:\'\"%/~+=-\[\]\(\)\{\}]'
    return re.sub(pattern, '', s)
parser = argparse.ArgumentParser(description='SSP')
parser.add_argument('--asr-gpu', action='store_true', default=False, help='asr-gpu')
parser.add_argument('--tts-gpu', action='store_true', default=False, help='tts-gpu')
args = parser.parse_args()
def quickstart_v2(
    project_id: str,
    audio_file: str,
) -> cloud_speech.RecognizeResponse:
    """Transcribe an audio file."""
    # Instantiates a client
    client = SpeechClient()

    # Reads a file as bytes
    with open(audio_file, "rb") as f:
        content = f.read()

    config = cloud_speech.RecognitionConfig(
        auto_decoding_config=cloud_speech.AutoDetectDecodingConfig(),
        language_codes=["en-US"],
        model="long",
    )

    request = cloud_speech.RecognizeRequest(
        recognizer=f"projects/{project_id}/locations/global/recognizers/_",
        config=config,
        content=content,
    )

    # Transcribes the audio into text
    response = client.recognize(request=request)

    for result in response.results:
        print(f"Transcript: {result.alternatives[0].transcript}")

    return response

def webm_to_wav(webm_path, wav_path, sampling_rate, channel):
    """
    webm 转 wav
    :param webm_path: 输入 webm 路劲
    :param wav_path: 输出 wav 路径
    :param sampling_rate: 采样率
    :param channel: 通道数
    :return: wav文件
    """
    # 如果存在wav_path文件，先删除。
    if os.path.exists(wav_path):  # 如果文件存在
        # 删除文件，可使用以下两种方法。
        os.remove(wav_path)
    # 终端命令
    command = "ffmpeg -loglevel quiet -i {} -ac {} -ar {} {}".format(webm_path, channel, sampling_rate, wav_path)
    # print('命令是：',command)
    # 执行终端命令
    os.system(command)
    
@app.route('/')
def index():
    return render_template('./index.html')

def process_audio_file(wav_path):
    if not args.asr_gpu:
        text = ''
        output = quickstart_v2('advance-anvil-399603', wav_path)
        for result in output.results:
            text = text + result.alternatives[0].transcript
    else:
        output = asr_executor(
            model='deepspeech2offline_librispeech',
            lang='en',
            sample_rate=16000,
            config=None,  # Set `config` and `ckpt_path` to None to use pretrained model.
            ckpt_path=None,
            audio_file=wav_path,
            force_yes=True,
            device=paddle.get_device()
        )
        text = '{}'.format(output)
    return text

@app.route('/upload-audio', methods=['POST'])
def upload_audio():
    file = request.files['audio']
    webm_path = 'wakeup/received_audio.webm'
    wav_path = 'wakeup/received_audio.wav'
    file.save(webm_path)
    webm_to_wav(webm_path, wav_path, 16000, 1)
    text = process_audio_file(wav_path)
    if ('hey' in text) and ('siri' in text.lower()):
        response = True
    else:
        response = False
    return jsonify({'response': response})
    
@app.route('/asr', methods=['POST'])
def asr():
    file = request.files['audio']
    webm_path = 'dialog/received_audio.webm'
    wav_path = 'dialog/received_audio.wav'
    file.save(webm_path)
    webm_to_wav(webm_path, wav_path, 16000, 1)
    transcription = process_audio_file(wav_path)
    
    return jsonify({'success': True, 'transcription': transcription})

@app.route('/chat', methods=['POST'])
def chat():
    data = request.get_json()
    text : str = data['text']
    print(text)
    data_to_send = {'query': text}

    # Send the POST request
    response = requests.post(llmurl, data=data_to_send)
    text = clean_string(response.text)
    return jsonify({'status_code': response.status_code, 'text': text})
    # return jsonify({'status_code': 200, 'text': 'good job!'})

@app.route('/tts', methods=['POST'])
def tts():
    data = request.get_json()
    text : str = data['text']
    output_name = 'dialog/response_audio.wav'
    if not args.tts_gpu:
        synthesis_input = texttospeech.SynthesisInput(text=text)

        # voice gender ("neutral")
        voice = texttospeech.VoiceSelectionParams(
            language_code="en-US", ssml_gender=texttospeech.SsmlVoiceGender.NEUTRAL
        )

        # Select the type of audio file you want returned
        audio_config = texttospeech.AudioConfig(
            audio_encoding=texttospeech.AudioEncoding.MP3
        )
        # Perform the text-to-speech request on the text input with the selected
        # voice parameters and audio file type
        response = client_tts.synthesize_speech(
            input=synthesis_input, voice=voice, audio_config=audio_config
        )
        # The response's audio_content is binary.
        with open(output_name, "wb") as out:
            # Write the response to the output file.
            out.write(response.audio_content)
            print(f'Audio content written to file {output_name}')
    else:
        wav_file = tts_executor(
            text=text,
            output=output_name,
            am='fastspeech2_ljspeech',
            am_config=None,
            am_ckpt=None,
            am_stat=None,
            spk_id=0,
            phones_dict=None,
            tones_dict=None,
            speaker_dict=None,
            voc='hifigan_vctk',
            voc_config=None,
            voc_ckpt=None,
            voc_stat=None,
            lang='en',
            device=paddle.get_device())
        print('Wave file has been generated: {}'.format(wav_file))
    return send_file(output_name, as_attachment=True)
    
        

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0", port=60340)
