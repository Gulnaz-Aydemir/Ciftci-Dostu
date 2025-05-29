-- Kullanıcılar tablosu
CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    phone VARCHAR(20),
    field_location TEXT
);

-- Araç bilgileri
CREATE TABLE devices (
    device_id SERIAL PRIMARY KEY,
    serial_number VARCHAR(50) UNIQUE NOT NULL,
    battery_level INTEGER CHECK (battery_level BETWEEN 0 AND 100),
    current_location TEXT,
    user_id INT REFERENCES users(user_id) ON DELETE CASCADE
);

-- Görev geçmişi
CREATE TABLE missions (
    mission_id SERIAL PRIMARY KEY,
    device_id INT REFERENCES devices(device_id) ON DELETE CASCADE,
    user_id INT REFERENCES users(user_id) ON DELETE CASCADE,
    start_point TEXT NOT NULL,
    end_point TEXT NOT NULL,
    mission_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Uyarılar
CREATE TABLE alerts (
    alert_id SERIAL PRIMARY KEY,
    device_id INT REFERENCES devices(device_id) ON DELETE CASCADE,
    alert_type VARCHAR(50), -- örn: 'Low Battery', 'Sensor Error'
    alert_message TEXT,
    alert_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Sensör verileri
CREATE TABLE sensor_data (
    data_id SERIAL PRIMARY KEY,
    device_id INT REFERENCES devices(device_id) ON DELETE CASCADE,
    temperature FLOAT,
    humidity FLOAT,
    soil_moisture FLOAT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

---

-- Kullanıcı ekle
INSERT INTO users (name, phone, field_location)
VALUES ('Mehmet Yılmaz', '+905321234567', 'Manisa, Alaşehir Köyü');

-- Cihaza bağla
INSERT INTO devices (serial_number, battery_level, current_location, user_id)
VALUES ('CD-TRM-001', 85, 'Tarımsal Yol 2. Parsel', 1);

-- Görev ekle
INSERT INTO missions (device_id, user_id, start_point, end_point)
VALUES (1, 1, 'Tarlanın Alt Kenarı', 'Depo Önü');

-- Uyarı ekle
INSERT INTO alerts (device_id, alert_type, alert_message)
VALUES (1, 'Low Battery', 'Batarya seviyesi %15 altına düştü.');

-- Sensör verisi kaydet
INSERT INTO sensor_data (device_id, temperature, humidity, soil_moisture)
VALUES (1, 28.5, 55.2, 0.35);