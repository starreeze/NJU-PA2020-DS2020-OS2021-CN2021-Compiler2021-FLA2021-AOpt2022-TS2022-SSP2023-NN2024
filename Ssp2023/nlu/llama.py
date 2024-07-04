# -*- coding: utf-8 -*-
# @Date    : 2023-12-15 16:01:34
# @Author  : Starreeze
"""deploy llama2-chat on single gpu"""

from __future__ import annotations
import os, sys, torch, logging

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))
import args
from transformers import LlamaForCausalLM, LlamaTokenizer

MODEL, TOKENIZER = None, None


class LlamaChat:
    def __init__(self, prompt, max_new_tokens) -> None:
        self.prompt = args.llama_sys_prompt.replace(args.llama_instruction_placeholder, prompt)
        self.max_new_tokens = max_new_tokens
        global MODEL, TOKENIZER
        if MODEL is None:
            init_llama()
        self.model, self.tokenizer = MODEL, TOKENIZER

    def infer(self, inputs: str) -> str:
        inputs = self.prompt + inputs
        logging.debug(f"LLM receives inputs: {inputs}")
        model_inputs = self.tokenizer(inputs, return_tensors="pt", return_token_type_ids=False).to(  # type:ignore
            "cuda:0"
        )
        output = self.model.generate(  # type:ignore
            **model_inputs,
            min_length=1,
            max_new_tokens=self.max_new_tokens,
            num_beams=1,
            temperature=1,
            top_p=0.9,
            repetition_penalty=1,
            length_penalty=1,
            do_sample=False,
        )
        output_tokens = output[0][len(model_inputs["input_ids"][0]) :]
        response = self.tokenizer.decode(output_tokens, skip_special_tokens=True)  # type:ignore
        return response


def init_llama():
    global MODEL, TOKENIZER
    MODEL = LlamaForCausalLM.from_pretrained(
        args.llama_path, local_files_only=True, torch_dtype=torch.float16, device_map={"": 0}
    )
    TOKENIZER = LlamaTokenizer.from_pretrained(args.llama_path, use_fast=True, local_files_only=True)


def main():
    llama = LlamaChat("", 200)
    while True:
        print(llama.infer(input("> ")))


if __name__ == "__main__":
    main()
