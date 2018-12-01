# Исторические данные брокера Binary.com

### Описание
Данный репозиторий содержит исторические данные котировок и процентов выплат брокера Binary.com

### Структура файлов исторических данных

Папка proposal_data содержит файлы с процентами выплат для следующих параметров:
* *Длительность опциона 3 минуты*
* *Опцион PUT и CALL*

Параметры можно просмотреть в файле *proposal_data/parameters.json*

```json
{
    "amount": 10.0,
    "currency": "USD",
    "duration": 3,
    "duration_uint": 2,
    "symbols": [
        "WLDAUD",
        "WLDEUR",
        "WLDGBP",
        "WLDUSD",
        "frxAUDCAD",
        "frxAUDCHF",
        "frxAUDJPY",
        "frxAUDNZD",
        "frxAUDUSD",
        "frxEURAUD",
        "frxEURCAD",
        "frxEURCHF",
        "frxEURGBP",
        "frxEURJPY",
        "frxEURNZD",
        "frxEURUSD",
        "frxGBPAUD",
        "frxGBPCAD",
        "frxGBPCHF",
        "frxGBPJPY",
        "frxGBPNZD",
        "frxNZDUSD",
        "frxUSDCAD",
        "frxUSDJPY"
    ]
}
```

* *amount* - ставка, используемая для расчета процентов выплат
* *currency* - валюта счета
* *duration* - экспирация опциона, зависит также от параметра *duration_uint*, в примере это 3 минуты
* *duration_uint* - единица измерения экспирации опциона, 2 соответствует минутам
* *symbols* - массив валютных пар

Каждая строка файлов исторических данных процентов выплат содержит JSON строку. Пример JSON строки:

```json
{"data":[{"buy":0.869,"sell":0.867,"symbol":"WLDAUD"},{"buy":0.846,"sell":0.825,"symbol":"WLDEUR"},{"buy":0.816,"sell":0.855,"symbol":"WLDGBP"},{"buy":0.835,"sell":0.835,"symbol":"WLDUSD"},{"buy":0.764,"sell":0.875,"symbol":"frxGBPNZD"},{"buy":0.869,"sell":0.769,"symbol":"frxNZDUSD"},{"buy":0.805,"sell":0.871,"symbol":"frxUSDCAD"},{"buy":0.869,"sell":0.869,"symbol":"frxUSDJPY"}],"time":1543553947}
```

* *time* - содержит timestamp, время GMT
* *data* - массив данных
* *buy* - процент выплат за сделку buy (PUT), от 0 до 1.0 (иногда может быть больше 1.0, 1.0 соответствует 100% выплате брокера)
* *sell* - процент выплат за сделку sell (CALL), от 0 до 1.0 (иногда может быть больше 1.0, 1.0 соответствует 100% выплате брокера)
* *symbol* - имя валютной пары (форекс валютные пары у брокера Binary начинаются с приставки *frx*)

Одна строка файла соответствует одной секунде. 
Один файл соответствует одному дню. 
Файлы обновляются в репозитории каждый торговый день в 00:00 GMT

### Полезные ссылки

C++ библиотека Websocket API брокера Binary.com [https://github.com/NewYaroslav/binary-cpp-api](https://github.com/NewYaroslav/binary-cpp-api)

Сайт брокера - [https://www.binary.com/](https://www.binary.com/)

С++ библиотека для парсинга JSON [https://github.com/nlohmann/json](https://github.com/nlohmann/json)

### Автор

elektro yar [electroyar2@gmail.com](electroyar2@gmail.com)
