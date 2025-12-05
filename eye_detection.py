import cv2
import numpy as np
import requests
import time

# URL вашего видеопотока ESP32-CAM
# Замените на IP вашей камеры
CAMERA_URL = "http://192.168.233.153:81/stream"

def test_camera_connection():
    """Тестируем подключение к камере"""
    print("🔗 Подключаемся к камере...")
    
    # Создаем объект для захвата видео
    cap = cv2.VideoCapture(CAMERA_URL)
    
    if not cap.isOpened():
        print("❌ Не удалось подключиться к камере!")
        print("Проверьте:")
        print("1. IP-адрес камеры")
        print("2. Что камера передает видеопоток")
        print("3. Сетевые настройки")
        return
    
    print("✅ Камера подключена!")
    
    # Загружаем классификаторы для обнаружения лиц и глаз
    print("📦 Загружаем модели детектирования...")
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
    eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')
    
    if face_cascade.empty() or eye_cascade.empty():
        print("❌ Не удалось загрузить модели детектирования!")
        return
    
    print("✅ Модели загружены!")
    print("🎯 Нажмите 'q' для выхода из программы")
    
    while True:
        # Захватываем кадр
        ret, frame = cap.read()
        
        if not ret:
            print("❌ Не удалось получить кадр")
            break
        
        # Конвертируем в оттенки серого для детектирования
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Обнаруживаем лица
        faces = face_cascade.detectMultiScale(gray, 1.3, 5)
        
        for (x, y, w, h) in faces:
            # Рисуем прямоугольник вокруг лица
            cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
            
            # Область интереса (лицо) в оттенках серого и цветном
            roi_gray = gray[y:y+h, x:x+w]
            roi_color = frame[y:y+h, x:x+w]
            
            # Обнаруживаем глаза внутри лица
            eyes = eye_cascade.detectMultiScale(roi_gray)
            
            # Анализируем состояние глаз
            if len(eyes) == 0:
                status = "ГЛАЗА ЗАКРЫТЫ"
                color = (0, 0, 255)  # Красный
            else:
                status = "ГЛАЗА ОТКРЫТЫ"
                color = (0, 255, 0)  # Зеленый
            
            # Отображаем статус
            cv2.putText(frame, status, (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
            
            # Рисуем прямоугольники вокруг глаз
            for (ex, ey, ew, eh) in eyes:
                cv2.rectangle(roi_color, (ex, ey), (ex+ew, ey+eh), (0, 255, 0), 2)
        
        # Показываем кадр
        cv2.imshow('ESP32-CAM - Детектирование глаз', frame)
        
        # Выход по нажатию 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    # Освобождаем ресурсы
    cap.release()
    cv2.destroyAllWindows()
    print("👋 Программа завершена")

if __name__ == "__main__":
    test_camera_connection()