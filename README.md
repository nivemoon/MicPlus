# MicPlus

Утилита для управления микрофоном в Windows с поддержкой push-to-talk (PTT).

- **Режим рации (push-to-talk) и переключения микрофона назначаемыми клавишами**
- **Глобальные хуки** — кнопки PTT и микрофона ловят нажатия во всех приложениях

<img width="223" height="132" alt="tLUXOGprNZ" src="https://github.com/user-attachments/assets/bced0d4a-479d-4e3e-bc39-bcbf87eba2b1" />
<img width="252" height="146" alt="DQD153VS5F" src="https://github.com/user-attachments/assets/047613f9-6cbc-4e08-aead-5763780117e1" />
<img width="199" height="132" alt="VnrID1fTO5" src="https://github.com/user-attachments/assets/a50facb0-2e26-4580-84e4-b963f988a8c7" />


> English description is available somewhere down below.

💡 **Нужен только PTT для мыши?** Посмотрите [**PTT-FFN**](https://github.com/nivemoon/PPT-FFN) — лёгкая утилита только для режима рации.

## Установка

### 📥 Прямые ссылки

- [**MicPlus-Setup-1.6.1.exe**](https://github.com/nivemoon/micplus/releases/download/1.6.1/MicPlus-Setup-1.6.1.exe) — установочная версия 
- [**MicPlus_1.6.1_Portable.zip**](https://github.com/nivemoon/micplus/releases/download/1.6.1/MicPlus_1.6.1_Portable.zip) — портативная версия 

### Портативная версия
1. Скачать архив выше или из [Releases](https://github.com/nivemoon/micplus/releases)
2. Распаковать в любую папку
3. Запустить `MicPlus.exe`

### Установочная версия
1. Скачать установщик выше или из [Releases](https://github.com/nivemoon/micplus/releases)
2. Запустить и следовать инструкциям мастера установки

<details>
<summary>🔧 Самостоятельная сборка из исходников</summary>

Требуется MinGW-w64:

```bat
windres -i res.rc -o res.o
gcc -o MicPlus.exe *.c res.o -mwindows -lwinhttp -lwinmm -luuid -lcomctl32 -lole32 -lcomdlg32 -DUNICODE -D_UNICODE
```

Или используйте `Makefile`:

```bat
mingw32-make
```
</details>

<img width="527" height="420" alt="MicPlus_J5K129iety" src="https://github.com/user-attachments/assets/7754ab8d-ce36-44b8-ba85-f73000742215" />

## ✅ Фикс v1.6.1

- **Кириллица** устранена в хоткеях, для правильного отображения без кривой кодировки.
- **Быстрый РТТ** - помимо двойного клика, так же клавиша активировалась при долгом удержании. Больше нет.


## ✅ Что нового в v1.6

- **Синхронизация с системой** — утилита читает фактическое состояние микрофона через `IAudioEndpointVolume` и синхронизирует свой переключатель с ним
- **Новая система звуков** — звуки, регулятор громкости и звуковые схемы
- **Низкоуровневые хуки** — новый механизм обработки, заметно меньше отставаний при высокой нагрузке процессора/видеокарты
- **Убран Win из доступных модификаторов** — слишком чувствительная и важная кнопка для системы
- **Ускорение запуска** — оптимизирован запуск `micctl.exe`
- **Режим PTT и Быстрый PTT** — новые режимы работы микрофона
- **Комбинация из двух клавиш** — новое сочетание клавиш для активации (Микрофон и Режим PTT)
- **Новый дизайн** — обновлённое оформление интерфейса (рация, громкость)
- **Исправление правого Alt (РУС)** — полное равноправие между альтами
- **Механизм «проверки» звука в системе** — Ctrl + Alt + Shift + (???). Каркас стелс плеера.

<details>
  
<summary>📜 Изменения прошлых версий</summary>

### v1.5 — 2026-09-24
- 🔧 Модульная структура проекта (1 файл → 17 модулей)
- 🆕 Встроенный updater для проверки обновлений
- 🆕 Возможность удаления настроек через меню "О программе"
- 🆕 Опция скрытия иконки из трея (сочетание `Ctrl+Alt+Shift+M`)
- 🐛 Исправления и улучшения
- Раздельные варианты: портативная и установочная версии (Inno Setup Compiler)

### v1.4
- Добавлены низкоуровневые хуки — ускорена скорость срабатывания, добавлена поддержка игр (League of Legends, PUBG, Throne and Liberty и др.)
- Перестроен функционал выбора и назначения клавиш — теперь можно выбрать одну клавишу для обоих режимов
- Управление звуком упрощено до комбинации `Ctrl+F9`
- Исправлено отображение в диспетчере задач — иконка меняется вместе с иконкой в трее

### v1.33_afix
- Изменён механизм автозагрузки — теперь используется безопасный метод создания/удаления ярлыка в папку автозагрузок (`%userprofile%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup`)
- Исправлена ложная индикация при автозапуске с системой

### v1.3
- Переписаны названия меню и строк интерфейса
- Добавлено: автозапуск с системой, звуковые индикаторы, системные уведомления
- Новые иконки: чёрная, белая, красная (при отключенном микрофоне)
- Автоопределение цветовой темы Windows + ручное переключение

</details>

## Подробнее о программе

### Режим РТТ (push-to-talk)
- Активируемое состояние, при котором функционирует кнопка вызова рации. Включен по умолчанию.

**Кнопка Рации**
- Выбор из боковых кнопок мыши и тильды
- По умолчанию: нижняя боковая кнопка мыши (MB5)
- Допустимо назначение любой клавиши

### Состояние микрофона
- Выбор из F9, F10, тильды или любой другой клавиши
- Назначение: клавиша, две клавиши или сочетание с модификатором (Ctrl/Alt/Shift)
- По умолчанию: F10

### Быстрый РТТ (он же Пассивный РТТ)
- Активируемое состояние, в котором допустимо выбрать двойной клик клавиши для активации РТТ (держать их при этом не надо). Аналогичен по функционалу переключателю микрофона, но требует Режим РТТ (и использует соответствующие сигналы). Выключен по умолчанию.

### Индикаторы
- Иконка в трее (красный значок при выключенном микрофоне)
- Звуковые сигналы для обоих режимов
- Системные уведомления
- Отдельные иконки для светлой и тёмной темы Windows

### Настройки
- Сохранение в `micplus.ini` (`%USER%\AppData\Roaming\MicPlus`)
- Два языка интерфейса: **русский / английский**

## Использование

**Правый клик по иконке в трее (в скобках указаны значения по умолчанию):**
- Кнопка вызова рации РТТ (MB5)
- Включение режима PTT (Alt + P)
- Включение микрофона (F10)
- Сочетения клавиш
- Настройки 
- О программе (обновления и удаление файла ini)

> **Режим PTT (push-to-talk / рация):** при зажатой кнопке микрофон включается, при отпускании — выключается.
> 
> **Включение/отключение микрофона** работает независимо от режима PTT. Кнопка рации активирует РТТ и автоматически выключает микрофон после отпускания. 

## Лицензия

Проект распространяется по лицензии MIT. См. файл [`LICENSE`](LICENSE).

---

## English summary

**MicPlus** is a small Windows tray utility that gives you global control over your microphone: **mute / unmute / push-to-talk (PTT)**.

It consists of two parts:

- `micplus.exe` — tray app with an icon, PTT mode and global hotkeys.
- `micctl.exe` — console helper that directly toggles the microphone state.

### Features

- **Push-to-talk (PTT)**:
  - mouse side button (MB5)
  - any keyboard key
- **Toggle hotkey**:
  - F10 by default
  - any custom combination (key or Ctrl/Alt/Shift + key)
- **Sound toggle**:
  - `Ctrl+F9` by default
  - any custom combination
- **Tray icon** indicates microphone state:
  - separate icons for mic on / mic off / dark & light themes
- Settings saved to `micplus.ini` (hotkeys, language, PTT mode)
- UI languages: **Russian / English**
- **HideTrayIcon** option to hide tray icon (`Ctrl+Alt+Shift+M`)
- **Built-in updater** for checking updates

### Installation

1. Download the latest release from [Releases](https://github.com/nivemoon/micplus/releases).
2. Put `micplus.exe` and `micctl.exe` into the same folder.
3. Run `micplus.exe` — a MicPlus icon will appear in the system tray.
4. Configure hotkeys and PTT mode from the tray context menu.

### Build

See the Russian section above for a MinGW / gcc build example.

---

Developed by **Nivemoon Labs**.

Copyright © 2026 Nivemoon Labs
