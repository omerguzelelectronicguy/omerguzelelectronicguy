#!/bin/bash
DOWNLOADS_DIR="$HOME/Downloads"
SCRIPT_NAME="Download Organizer Daemon"

sleep 15;
notify-send -i system-run "$SCRIPT_NAME" "$0 Çalışıyor."
# Klasörü izlemeye al
inotifywait -m "$DOWNLOADS_DIR" -e moved_to -e close_write | while read path action file; do
    # Dosya uzantısını küçük harfe çevirerek al
    ext="${file##*.}"
    ext=$(echo "$ext" | tr 'A-Z' 'a-z')
    
    # Hedef klasör ve tetiklenme durumunu kontrol edecek değişkenler
    TARGET_DIR=""
    
    case "$ext" in
        stl|blend)
            TARGET_DIR="$HOME/Documents/3d/downloads"
            ;;
        gguf)
            TARGET_DIR="/depo/models"
            ;;
    esac

    # Eğer eşleşen bir uzantı varsa taşı ve bildirim gönder
    if [ ! -z "$TARGET_DIR" ]; then
        mkdir -p "$TARGET_DIR"
        mv "$DOWNLOADS_DIR/$file" "$TARGET_DIR/"
        
        # Sistem bildirimi gönder (Script adı, dosya adı ve hedef klasör ile)
        notify-send -i system-run "$SCRIPT_NAME" "$0 Çalışıyor.\n<b>Dosya Taşındı:</b> $file\n<b>Hedef:</b> $TARGET_DIR"
    fi
done
