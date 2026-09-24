# MicPlus

Утилита для управления микрофоном в Windows с поддержкой push-to-talk (PTT).

- **Режим рации и переключения микрофона назначаемыми клавишами**
- **Глобальный звук** — включение/выключение системного звука
- **Глобальные хуки** — кнопки PTT и микрофона ловят нажатия во всех приложениях

<img width="230" height="124" alt="MicPlus tray" src="https://github.com/user-attachments/assets/7494c5d3-9372-4282-b74f-9d30aa2982f8" />
<img width="218" height="95" alt="MicPlus menu" src="https://github.com/user-attachments/assets/004a302d-aca5-47e5-8b86-e8757774e923" />
<img width="270" height="168" alt="MicPlus settings" src="https://github.com/user-attachments/assets/9525d8dd-e785-41fd-a635-c674d4c38ef1" />

> English description is available somewhere down below.

💡 **Нужно только PTT для мыши?** Посмотрите [**PTT-FFN**](https://github.com/nivemoon/PPT-FFN) — лёгкая утилита только для режима рации.

## Установка

### Портативная версия
1. Скачайте архив из [Releases](https://github.com/nivemoon/micplus/releases)
2. Распакуйте в любую папку
3. Запустите `MicPlus.exe`

### Установочная версия
1. Скачайте установщик из [Releases](https://github.com/nivemoon/micplus/releases)
2. Запустите и следуйте инструкциям мастера установки

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

<img width="525" height="393" alt="MicPlus settings window" src="https://github.com/user-attachments/assets/ae27ca55-207a-4ed0-b33b-53b8aa65e89e" />

## ✅ Что нового в v1.5

- **HideTrayIcon** — опция скрытия иконки из трея (сочетание `Ctrl+Alt+Shift+M`)
- **Встроенный updater** — проверка обновлений внутри приложения
- **Удаление настроек** — очистка `micplus.ini` через меню "О программе"
- **Модульная структура** — рефакторинг кода (1 файл → 17 модулей)
- **Раздельные версии** — портативная и установочная (Inno Setup Compiler)

<details>
<summary>📜 Changelog</summary>

### v1.5 — 2026-09-24
- 🔧 Модульная структура проекта (1 файл → 17 модулей)
- 🆕 Встроенный updater для проверки обновлений
- 🆕 Опция скрытия иконки из системного трея (HideTrayIcon)
- 🆕 Возможность удаления настроек через меню "О программе"
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

### Режим рации (PTT)
- Выбор из боковых кнопок мыши и тильды
- По умолчанию: нижняя боковая кнопка мыши (MB5)
- Допустимо назначение любой клавиши

### Состояние микрофона
- Выбор из F9, F10, тильды или любой другой клавиши
- Назначение: любая клавиша или сочетание с модификатором (Ctrl/Alt/Shift/Win)
- По умолчанию: F10

> **Примечание:** Из-за особенностей Windows и русской раскладки, правый Alt может назначиться как `Ctrl+Alt`.

### Индикаторы
- Иконка в трее (красный значок при выключенном микрофоне)
- Звуковые сигналы для обоих режимов
- Системные уведомления
- Отдельные иконки для светлой и тёмной темы Windows

### Настройки
- Сохранение в `micplus.ini` (`%USER%\AppData\Roaming\MicPlus`)
- Два языка интерфейса: **русский / английский**

## Использование

**Правый клик по иконке в трее:**
- Включение режима PTT (MB5 или кастомная клавиша)
- Включение микрофона (F10 / своё)
- Выбор и переназначение сочетаний клавиш
- Настройки
- О программе (обновления и удаление файла ini)

> **Режим PTT (push-to-talk / рация):** при зажатой кнопке микрофон включается, при отпускании — выключается.
> 
> **Включение/отключение микрофона** работает независимо от режима PTT. Включение режима PTT автоматически выключает микрофон.

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
  - any custom combination (key or Ctrl/Alt/Shift/Win + key)
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
