
import subprocess
from bs4 import BeautifulSoup
import os
import os.path
import re
import string


def get_words(from_letters: str, to_letters: str):
    words = []
    with open(r'c:\project\html\nihon_jiten\dict\dict_words_edited.txt') as file:
        for word in file.readlines():
            word = word.rstrip()
            if from_letters <= word < to_letters:
                words.append(word)
    return words


def grab_article(word: str, known_articles: {str}):
    app = r'c:\project\html\nihon_jiten\cpp\utools_export\Release\utools_export.exe'
    out_file = r'c:\project\html\nihon_jiten\python\output\dict_www.html'
    subprocess.run([app, 'word', word, out_file])
    if not os.path.exists(out_file):
        print(f'No such word: {word}')
        return None
    with open(out_file, mode='rb') as file:
        content = file.read()
        soup = BeautifulSoup(content, 'html.parser')
        #print(soup)

        out = []
        for entry in soup.find_all('div', class_='DICTENTRY'):
            #print(entry.h3)
            title = entry.h3.get_text()
            title = title.replace('·', '')
            entry.h3.string = title
            if title in known_articles:
                continue
            out.append(entry)
            known_articles.add(title)
            for para in entry.find_all('p'):
                break
                print(para)
    os.unlink(out_file)
    return out


def get_html_header(title: str):
    html_header = f"""
    <!DOCTYPE html>
    <html lang="jp">
      <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>{title}</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
              crossorigin="anonymous">
        <link rel="stylesheet" href="dict.css">
      </head>
      <body data-bs-theme="dark">
      
      <div class="container">
      <h1>{title}</h1>
    """
    return html_header


html_footer = """

 </div>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js" crossorigin="anonymous"></script>
  </body>
</html>
"""


def post_process(body: str):
    body = re.sub(r'([;,.])(?=\S)', r'\1 ', body)
    body = body.replace('·', '')  # syllable delimiter
    for part_of_speech in ('名', '動', '他', '自', '形', '副', '前'):
        body = body.replace(f'[{part_of_speech}]', f' <span class="pos">[{part_of_speech}]</span> ')
    body = body.replace('class="DICTENTRY"', 'class="dictentry"')
    return body


def process_words(file_name, title):
    from_letters, to_letters = file_name.lower().split('-')
    words_todo = get_words(from_letters, to_letters)
    #file_name = file_name[:-1] + chr(ord(file_name[-1]) - 1)
    known_articles = set()
    res = []
    for word in words_todo:
        article_entries = grab_article(word, known_articles)
        if article_entries:
            res.extend(article_entries)
    body = '\n'.join(str(item) for item in res)
    body = post_process(body)
    whole = get_html_header(title) + body + html_footer
    with open(f'{title.lower()}.html', mode='w', encoding='utf-8') as file:
        file.write(whole)

"""
Aa-ag [Aa-af]
Ag-ao [Ag-an]
Ao-au [Ao-at]
Au-ba [Au-az]

Ba-bf [Ba-be]
Bf-bs bf-br
Bs-ca Bs-Bz

Ca-cf cf-ce
cf-cm cf-cl
cm-cp cm-co
cp-da cp-cz

Da-df
df-dk
dk-ea

ea-en
en-ew
ew-fa

fa-ga
ga-ha
---
ha-ia
ia-ja

ka-la
la-ma
ma-na
na-oa
"""


for letter in list(string.ascii_lowercase)[16:]:
    next_letter = chr(ord(letter) + 1)
    process_words(file_name=f'{letter}a-{next_letter}a', title=letter)

