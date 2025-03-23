#pyright: strict

import translators as ts
# from typing import Final

class Translator:
    
    # @final
    MAX_LENGTH:int = 1000

    @staticmethod
    def read_file(file_path: str) -> str:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        return content

    @staticmethod
    def split_text_into_chunks(text: str, max_length: int) -> list[str]:
        lines = text.splitlines()
        chunks: list[str] = []
        current_chunk = ""

        for line in lines:
            if len(current_chunk) + len(line) + 1 > max_length:
                chunks.append(current_chunk)
                current_chunk = line
            else:
                if current_chunk:
                    current_chunk += "\n" + line
                else:
                    current_chunk = line

        if current_chunk:
            chunks.append(current_chunk)

        return chunks

    @staticmethod
    def translate(file_path: str) -> str:
        translated_text: str = Translator.read_file(file_path)
        trans_text = ""
        for line in Translator.split_text_into_chunks(translated_text, Translator.MAX_LENGTH):
            trans_text += ts.translate_text(line, from_language='en', to_language='zh')
        return trans_text


if __name__ == "__main__":
    result:str = Translator.translate("d:\\Yumud\\scripts\\translator\\input.txt")
    print(result)