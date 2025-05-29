import cv2
import mediapipe as mp
import serial
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 9600

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    max_num_hands=1, min_detection_confidence=0.7, min_tracking_confidence=0.7
)
mp_draw = mp.solutions.drawing_utils


def init_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Seri port {SERIAL_PORT} @ {BAUD_RATE} baud rate ile acildi.")
        time.sleep(2)
        return ser
    except serial.SerialException as e:
        print(f"HATA: Seri port {SERIAL_PORT} acilamadi: {e}")
        print(
            "Lutfen ESP32'nin bagli oldugundan ve dogru portun secildiginden emin olun."
        )
        print("Betik sonlandiriliyor.")
        exit()


def main():
    ser = init_serial()

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("HATA: Kamera acilamadi. Lutfen kameranizin bagli oldugundan emin olun.")
        ser.close()
        return

    print("Kamera acildi. El hareketlerini algilamak icin kameraya elinizi gosterin.")
    print(" 'q' tusuna basarak cikabilirsiniz.")

    last_command_time = time.time()
    command_interval = 1.0
    last_sent_command = None

    while cap.isOpened():
        success, img = cap.read()
        if not success:
            print("Kameradan goruntu alinamiyor.")
            break

        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        results = hands.process(img_rgb)
        current_command = None

        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                mp_draw.draw_landmarks(img, hand_landmarks, mp_hands.HAND_CONNECTIONS)

                thumb_tip = hand_landmarks.landmark[mp_hands.HandLandmark.THUMB_TIP]
                index_finger_tip = hand_landmarks.landmark[
                    mp_hands.HandLandmark.INDEX_FINGER_TIP
                ]
                middle_finger_tip = hand_landmarks.landmark[
                    mp_hands.HandLandmark.MIDDLE_FINGER_TIP
                ]
                index_finger_mcp = hand_landmarks.landmark[
                    mp_hands.HandLandmark.INDEX_FINGER_MCP
                ]

                if (
                    thumb_tip.y < index_finger_mcp.y
                    and index_finger_tip.y > index_finger_mcp.y
                    and middle_finger_tip.y
                    > hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_MCP].y
                ):
                    current_command = "forward"
                elif (
                    index_finger_tip.y < index_finger_mcp.y
                    and middle_finger_tip.y
                    < hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_MCP].y
                    and hand_landmarks.landmark[mp_hands.HandLandmark.RING_FINGER_TIP].y
                    < hand_landmarks.landmark[mp_hands.HandLandmark.RING_FINGER_MCP].y
                    and hand_landmarks.landmark[mp_hands.HandLandmark.PINKY_TIP].y
                    < hand_landmarks.landmark[mp_hands.HandLandmark.PINKY_MCP].y
                ):
                    current_command = "stop"
                else:
                    pass
        else:
            current_command = "stop"

        current_time = time.time()
        if current_command is not None and (
            current_command != last_sent_command
            or (current_command == "stop" and last_sent_command != "stop")
            or (current_time - last_command_time > command_interval)
        ):
            try:
                command_to_send = current_command + "\n"
                ser.write(command_to_send.encode("utf-8"))
                print(f"Komut gonderildi: {current_command}")
                last_sent_command = current_command
                last_command_time = current_time
            except serial.SerialException as e:
                print(f"Seri yazma hatasi: {e}")
                ser.close()
                print("Seri baglantisi koptu. Betik sonlandiriliyor.")
                break
            except Exception as e:
                print(f"Beklenmedik bir hata olustu: {e}")

        cv2.imshow("El Hareketi Tanima - Ciftci Dostu Projesi", img)

        if cv2.waitKey(5) & 0xFF == ord("q"):
            print("Cikis yapiliyor...")
            break

    if ser.is_open:
        try:
            ser.write(b"stop\n")
            print("Cikista 'stop' komutu gonderildi.")
        except:
            pass
        ser.close()
        print("Seri port kapatildi.")
    cap.release()
    cv2.destroyAllWindows()
    print("Kamera ve pencereler kapatildi.")


if __name__ == "__main__":
    main()
