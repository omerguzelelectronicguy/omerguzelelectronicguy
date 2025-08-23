import os
import sys
import re
from collections import defaultdict

def clean_filename(filename):
    """
    Dosya adını temizleyerek kelimeleri listeye döndürür.
    """
    # Dosya uzantısını sil
    name_without_ext = os.path.splitext(filename)[0]
    
    # Parantez içindeki ifadeleri ve özel karakterleri sil
    cleaned_name = re.sub(r'(\(.*?\)|\[.*?\])', '', name_without_ext)
    
    # Harf, rakam ve boşluk dışındaki her şeyi boşlukla değiştir
    cleaned_name = re.sub(r'[^a-zA-ZçğıiöşüÇĞIİÖŞÜ0-9 ]', ' ', cleaned_name)
    
    # Tüm harfleri küçük yap ve birden fazla boşluğu tek boşluğa indir
    words = cleaned_name.lower().split()
    
    # Anlamsız kelimeleri filtrele
    ignore_words = {'feat', 'featuring', 'official', 'video', 'audio', 'lyrics', 'live', 'konser',
                    'remix', 'acoustic', 'version', 'prod', 'mix', 'master', 'cover', 'karaoke',
                    'edit', 'club', 'extended', 'original', 'full', 'single', 'explicit', 'clean',
                    'radio', 'remastered', 'remaster', 'demo', 'session', 'performance', 'clip', 'kbps'}
    
    # En az 3 harfli olan ve anlamsız olmayan kelimeleri döndür
    return [word for word in words if len(word) >= 3 and word not in ignore_words]

def find_music_files(directory):
    """
    Verilen dizindeki tüm müzik dosyalarını bulur.
    """
    music_files = []
    supported_exts = {'.mp3', '.flac', '.wav', '.m4a', '.aac', '.ogg'}
    
    for dirpath, _, filenames in os.walk(directory):
        for filename in filenames:
            if os.path.splitext(filename)[1].lower() in supported_exts:
                music_files.append(os.path.join(dirpath, filename))
    return music_files

def main():
    if len(sys.argv) != 3:
        print("Kullanım: python3 script.py <dizin_yolu> <minimum_ortak_kelime_sayısı>")
        print("Örnek: python3 script.py /home/user/muzik 3")
        sys.exit(1)

    music_dir = sys.argv[1]
    min_common_words = int(sys.argv[2])

    if not os.path.isdir(music_dir):
        print(f"Hata: '{music_dir}' dizini bulunamadı!")
        sys.exit(1)

    print("Müzik dosyaları taranıyor...")
    print(f"Dizin: {music_dir}")
    print(f"Minimum ortak kelime sayısı: {min_common_words}")
    print("----------------------------------------")

    files = find_music_files(music_dir)
    total_files = len(files)
    print(f"Toplam {total_files} müzik dosyası bulundu.")

    if total_files < 2:
        print("Eşleştirme için yeterli dosya yok.")
        sys.exit(0)

    # Dosya adlarını temizleyip kelimeleri ayır
    file_words = {}
    for filepath in files:
        filename = os.path.basename(filepath)
        file_words[filepath] = set(clean_filename(filename))

    print("Karşılaştırma başlıyor...")
    match_count = 0
    
    # Tüm dosya çiftlerini karşılaştır
    for i in range(total_files):
        for j in range(i + 1, total_files):
            file1 = files[i]
            file2 = files[j]

            words1 = file_words[file1]
            words2 = file_words[file2]

            # Kesişim için 'set' veri yapısını kullanmak çok hızlıdır.
            common_words = words1.intersection(words2)
            common_count = len(common_words)

            if common_count >= min_common_words:
                match_count += 1
                print("\n=== EŞLEŞME {} ===".format(match_count))
                print(f"Ortak kelime sayısı: {common_count}")
                print(f"Dosya 1: {file1}")
                print(f"Dosya 2: {file2}")
                print("Ortak kelimeler:", ", ".join(sorted(list(common_words))))
                print("----------------------------------------")
    
    print("\n=== ÖZET ===")
    print(f"Toplam dosya: {total_files}")
    print(f"Bulunan eşleşme: {match_count}")
    print("İşlem tamamlandı.")

if __name__ == "__main__":
    main()