
def grab_word_list():
    start_word_letters = set()
    inter_word_letters = set()
    for i in range(ord('a'), ord('z') + 1):
        start_word_letters.add(i)
        inter_word_letters.add(i)

    for i in range(ord('0'), ord('9') + 1):
        inter_word_letters.add(i)
    inter_word_letters.add(ord('_'))
    words = set()
    with open(r'e:\Program Files (x86)\Kodansha Encyclopedia of Japan\lib\dictix.udb', mode='rb') as file:
        word: bytearray = None

        def finish_word():
            nonlocal word
            if word is None:
                return
            words.add(word.decode(encoding='ascii'))
            word = None

        for ch in file.read():
            if word is None:
                if ch in start_word_letters:
                    word = bytearray()
                    word.append(ch)
            else:
                if ch in inter_word_letters:
                    word.append(ch)
                else:
                    finish_word()
        finish_word()
    arr = list(words)
    arr = sorted(arr)
    print(len(arr))
    #print(arr[0:100])
    with open('dict_words.txt', mode='w', encoding='utf8') as file:
        for word in arr:
            file.write(word)
            file.write('\n')

grab_word_list()
