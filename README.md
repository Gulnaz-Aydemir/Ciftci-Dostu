#  Çiftçi Dostu – Otonom Tarımsal Ürün Taşıma Aracı

Tarım sektöründe ürün taşıma süreçlerini akıllı hale getirmek üzere geliştirilen **Çiftçi Dostu**, yapay zeka ve otonom navigasyon teknolojileri ile donatılmış, çevre dostu ve verimli bir taşıma çözümüdür. Küçük ve orta ölçekli çiftçilerin verimliliğini artırmak ve iş gücü maliyetlerini azaltmak için tasarlanmıştır.

##  Projenin Amacı

Çiftçi Dostu, ürünlerin tarladan taşınması sırasında yaşanan:

- İş gücü kaybı
- Zaman kaybı
- Maliyet artışı

gibi sorunları ortadan kaldırmayı hedefleyen bir yapay zeka destekli taşıma aracıdır. Güneş enerjisiyle çalışır, IoT teknolojileriyle entegredir ve uzaktan kontrol edilebilir.

##  Özellikler

###  Fonksiyonel Gereksinimler

- Otonom rota belirleme ve taşıma
- Engel algılama ve kaçınma
- Uzaktan internet tabanlı kontrol
- Batarya durum bildirimi
- Şarj kulübesine otomatik giriş

###  Fonksiyonel Olmayan Gereksinimler

- 6–8 saat kesintisiz çalışma
- Maksimum 3 saat şarj süresi
- Güneşli/yağmurlu hava koşullarına dayanıklılık
- Mobil uyumlu kullanıcı arayüzü
- Güvenli bağlantı altyapısı

##  Hedef Kullanıcı Profilleri

- Küçük ölçekli çiftçiler
- Tarım işletmesi sahipleri
- Belediye tarım hizmetleri
- Tarım kooperatifleri
- Tarım danışmanları

##  Yazılım Geliştirme Süreci

Proje, **Scrum** metodolojisi ile 7 sprint boyunca yönetilmiştir. Her sprint sonunda çalışan bir modül test edilerek sürekli iyileştirme sağlanmıştır. Bu metodoloji sayesinde:

- Saha ihtiyaçlarına hızlı cevap verildi
- Kullanıcı geri bildirimleri entegre edildi
- Takım içi iletişim ve koordinasyon sağlandı

##  Kullanılan Teknolojiler

| Alan        | Teknoloji                               |
| ----------- | --------------------------------------- |
| Programlama | Python, Arduino C                       |
| Yapay Zeka  | Görüntü işleme, otonom karar sistemleri |
| Donanım     | GPS, DHT22, HL69 sensörleri             |
| Kontrol     | IoT, uzaktan bağlantı sistemleri        |
| Arayüz      | Web + Mobil destekli kullanıcı paneli   |
| Enerji      | Güneş panelleri, batarya sistemi        |

##  Sistem Bileşenleri

- **Gerçek Zamanlı Takip**: GPS ile konum takibi
- **Görev Planlama**: Başlangıç ve varış noktası belirleme
- **Sensör Paneli**: Toprak nemi, hava durumu verileri
- **Bildirim Sistemi**: Batarya azaldığında SMS/uyarı
- **UI/UX**: Mobil uyumlu, sembol tabanlı arayüz

##  Sistem Mimarisi

- Mikrodenetleyici (Arduino)
- Sensörler (Nem, sıcaklık, engel algılama)
- Otonom kontrol algoritmaları
- Gömülü sistem üzerinde çalışan AI destekli modüller
- Web/mobil arayüz için REST API desteği

##  Literatür ve Karşılaştırmalı Analiz

| Sistem                   | Avantaj                                        | Dezavantaj                               |
| ------------------------ | ---------------------------------------------- | ---------------------------------------- |
| **Çiftçi Dostu**         | Otonom, IoT, güneş enerjili, her araziye uygun | Geliştirme süreci devam ediyor           |
| IoT Sulama Sistemi       | Su tasarrufu                                   | Taşıma yapmaz                            |
| Endüstriyel Raylı Taşıma | Ağır yük, sabit hat                            | Mobil değil, tarım arazisine uygun değil |

##  Gelecek Planları

- Robotik kol entegrasyonu ile otomatik yükleme
- Diğer sektörlerde kullanıma yönelik yeni varyantlar (inşaat, lojistik vb.)
- Gelişmiş yapay zeka destekli karar sistemleri
- Tamamen otonom tarımsal operasyon yönetimi

##  Bağlantılar

- Proje Sahibi: **Gülnaz Aydemir** – [220204019@ostimteknik.edu.tr](mailto:220204019@ostimteknik.edu.tr)
- GitHub Repository: [github.com/Gulnaz-Aydemir/Ciftci-Dostu](https://github.com/Gulnaz-Aydemir/Ciftci-Dostu)

##  Kaynakça

- Özgüven, M. M., Emnoğlu, M. B., & Çolak, A. (2024). _Tarımda Otonom Araçların Kullanımı._
- Özlüoymak, Ö. B. _Hassas Tarımda Kullanılan Otonom Sistemler._
- [OpenCV](https://opencv.org/)
- [TinyGPS++ Arduino Kütüphanesi](http://arduiniana.org/libraries/tinygpsplus/)

---

> “Çiftçi Dostu, yalnızca bir tarım aracı değil, tarımın geleceğini şekillendiren akıllı bir sistemdir.” 🌱
