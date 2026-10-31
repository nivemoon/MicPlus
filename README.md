# MicPlus

Утилита для управления микрофоном, аудиоустройствами и звуком в Windows.

- **Микрофон**: включение / выключение, push-to-talk, голосовая активация
- **Аудиоустройства**: быстрый выбор и переключение входа и выхода
- **Глобальные хоткеи**: работают в играх и других приложениях
- **Аварийное отключение**: мгновенно выключает микрофон, режим PTT и голосовую активацию
      
<img width="261" height="139" alt="N2JIWRLJAs" src="https://github.com/user-attachments/assets/b85e04c0-8569-4528-bc7b-2a36bcf2eb43" />
<img width="223" height="219" alt="lNGCOOmSgX" src="https://github.com/user-attachments/assets/e005254d-593c-4ba9-a924-d3027ddafd5c" />
<img width="309" height="146" alt="Q0hwW1HGHA" src="https://github.com/user-attachments/assets/8405d6d1-6bf9-4476-b617-e6b987e1c01a" />

> Скриншоты (здесь и далее) сделаны в Windows 10. В Windows 11 используется современный визуал.  
> Работа всех функций в более старых версиях Windows не гарантируется.

💡 [**PTT-FFN**](https://github.com/nivemoon/PPT-FFN) — лёгкая утилита для рации боковыми кнопками мыши. Подходит, если нужны только PTT-функции.

### 📥 Прямые ссылки

- [**MicPlus-Setup-1.7.exe**](https://github.com/nivemoon/MicPlus/releases/download/1.7/MicPlus-Setup-1.7.exe) — установочная версия
- [**MicPlus_1.7_Portable.zip**](https://github.com/nivemoon/MicPlus/releases/download/1.7/MicPlus_1.7_Portable.zip) — портативная версия

### Портативная версия

1. Скачайте архив выше или из [Releases](https://github.com/nivemoon/MicPlus/releases).
2. Распакуйте в любую папку.
3. Запустите `MicPlus.exe`.

### Установочная версия

1. Скачайте установщик выше или из [Releases](https://github.com/nivemoon/MicPlus/releases).
2. Запустите его и следуйте инструкциям мастера установки.
<details>
<summary>🔧 Самостоятельная сборка из исходников</summary>

Требуется MinGW-w64:

```bat
windres -i res.rc -o res.o
gcc -o MicPlus.exe *.c res.o -mwindows -lwinhttp -lwinmm -luuid -lcomctl32 -lole32 -lcomdlg32 -DUNICODE -D_UNICODE
```

Или `Makefile`:

```bat
mingw32-make
```
</details>

<img width="527" height="420" alt="изображение" src="https://github.com/user-attachments/assets/e620857c-b36c-4145-a065-edd343941226" />

Полная история изменений — в [CHANGELOG.md](CHANGELOG.md).

## Что нового в v1.7

- **Голосовая активация** — микрофон включается от голоса и выключается после паузы; быстрое включение и выключение сочетанием `V+U`
- **Аварийное отключение** — `Ctrl+Alt+F10` мгновенно выключает микрофон, режим PTT и голосовую активацию
- **Управление аудиоустройствами** — быстрый выбор, переключение и переименование устройств ввода и вывода
- **Bluetooth: без спящего режима** — помогает наушникам не уходить в сон и уменьшает задержку перед началом звука
- **Обновлённая звуковая система** — удалена схема «Classic» со звуками Discord (распространение в стороннем приложении не разрешено лицензией Discord). Оставлен один набор свободно распространяемых звуков; любой сигнал можно заменить своим через настройки
- **Новое хранение настроек** — файлы `micplus.ini` и `aliases.ini` создаются рядом с `MicPlus.exe`; если это невозможно, например в `Program Files`, они сохраняются в `AppData`. Работает для портативной и установочной версий
- **Уменьшен размер программы** — примерно в 10 раз, за счёт удаления тестовых аудиофайлов
- **Возможность отключения всех хоткеев**
- **Обновлённые меню и настройки**

## Подробнее о программе

### Режим PTT (push-to-talk / рация)

При удержании назначенной кнопки микрофон включается, при отпускании — выключается.

- Активируемый режим, в котором работает кнопка рации. Голосовая активация доступна только в этом режиме.
- Переназначение: **Горячие клавиши → Клавиши рации (PTT) → Режим PTT**
- Голосовая активация не отключает клавиши рации.

<img width="214" height="227" alt="Голосовая активация" src="https://github.com/user-attachments/assets/cffef5ec-1aa4-4f05-9e55-012fd5bdf2e0" />

> Голосовой режим позволяет настроить условия срабатывания под себя.

### Кнопка рации

- Выбор боковых кнопок мыши. Используется особый механизм: другие действия этих кнопок игнорируются.
- По умолчанию — нижняя боковая кнопка мыши. В некоторых мышах кнопки 4 и 5 могут меняться местами.
- Можно назначить любую клавишу. После выбора клавиши доступен вариант активации двойным нажатием вместо удержания.

### Состояние микрофона

Включение и отключение микрофона работает независимо от режима PTT.

- По умолчанию: `F10`
- Можно назначить одну клавишу, две клавиши или сочетание с `Ctrl` / `Alt` / `Shift`.

### Аудиоустройства

<img width="260" height="224" alt="Аудиоустройства" src="https://github.com/user-attachments/assets/50df2b9c-b0ba-4a53-8b21-a5fad158b795" />
<img width="256" height="97" alt="Быстрый выбор устройств" src="https://github.com/user-attachments/assets/1c726554-3a01-4d88-bde0-7872b105a279" />
<img width="309" height="73" alt="Хоткеи устройств" src="https://github.com/user-attachments/assets/1932dc68-f59b-45aa-a268-c62e53a24995" />

> Примечание: тройной модификатор `Ctrl+Alt+Shift+%символ%` отображается только как `%символ%`, чтобы не растягивать окно.

MicPlus определяет подключённые устройства и позволяет переключать их на лету.

- Можно назначить собственные хоткеи для устройств.
- Устройства можно переименовать; названия сохраняются в `aliases.ini`.
- Микрофоны можно скрыть из быстрого переключателя устройств.
- Здесь же находится параметр «Bluetooth: без спящего режима».

### Индикаторы и особенности

- Отдельные значки для светлой и тёмной темы Windows.
- Альтернативная иконка: вместо значка MicPlus можно показывать текущее аудиоустройство — наушники, динамики или TWS. Для каждого состояния предусмотрены свои значки.
- Звуковые сигналы и регулировка их громкости. 
- Все горячие клавиши можно отключить, кроме голосовой активации `V+U`.
- Системные уведомления и автозагрузка с системой.
- Включение / выключение системного звука и возможность полностью скрыть значок в трее (подсказка — в окне «О программе»).
- Проверка обновлений и сброс настроек через меню «О программе». Настройки ищутся рядом с программой и в папке AppData.
- Настройка действий для значка в трее:
  - **Левый клик**: быстрый выбор аудиоустройств, главное меню или без действия.
  - **Колесо мыши**: микрофон вкл/выкл, панель аудиоустройств или без действия.

> Примечание: назначение действия на колесо мыши доступно только в Windows 10. В Windows 11 колесо повторяет действие левого клика.

### Настройки

- Сохранение в `micplus.ini` и `aliases.ini`.
- Два языка интерфейса: русский и английский.

## Лицензия

Проект распространяется по лицензии MIT. См. файл [`LICENSE`](LICENSE).

---

---

## English

**MicPlus** is a Windows tray utility for microphone, audio device and sound control.

### Features

- Microphone mute / unmute, push-to-talk and quick PTT
- Voice activation with `V+U` toggle
- Emergency off with `Ctrl+Alt+F10`
- Audio device selection, switching and renaming
- Bluetooth no-standby mode
- Global hotkeys that work in games and other applications
- Tray icon with light and dark theme support
- Multimedia panel and configurable mouse button actions
- Russian and English interface

### Installation

Download MicPlus 1.7 from [Releases](https://github.com/nivemoon/MicPlus/releases).

- **Portable:** extract the archive and run `MicPlus.exe`
- **Installer:** run the setup file and follow the instructions

### Build

See the Russian section above for MinGW-w64 build instructions.

---

Developed by **Nivemoon Labs**.

Copyright © 2026 Nivemoon Labs
