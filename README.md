# Исторические данные брокера Binary.com

### Описание
Данный репозиторий содержит исторические данные котировок и процентов выплат брокера Binary.com 

* Файлы обновляются в репозитории каждый торговый день в 00:00 GMT
* Один файл соответствует одному дню, название файла формируется из даты
* Файлы содержат информацию о следующих валютных парах или индексов:

*WLDAUD*
*WLDEUR*
*WLDGBP*
*WLDUSD*
*frxAUDCAD*
*frxAUDCHF*
*frxAUDJPY*
*frxAUDNZD*
*frxAUDUSD*
*frxEURAUD*
*frxEURCAD*
*frxEURCHF*
*frxEURGBP*
*frxEURJPY*
*frxEURNZD*
*frxEURUSD*
*frxGBPAUD*
*frxGBPCAD*
*frxGBPCHF*
*frxGBPJPY*
*frxGBPNZD*
*frxNZDUSD*
*frxUSDCAD*
*frxUSDJPY*

### Структура файлов исторических данных по процентам выплат брокера

Папка proposal_data содержит файлы с процентами выплат для нескольких валютных пар.
Однин сэмпл файла или строка json файла соответствует одной секунде. 
Один файл соответствует одному дню, название файла формируется из даты.
Файлы обновляются в репозитории каждый торговый день в 00:00 GMT.
Параметры можно просмотреть в файле *proposal_data/parameters.json*, пример параметров:

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

Проценты выплат хранятся в двоичных файлах для уменьшения размера файлов. В начале файла есть заголовок-строка, оканчивающееся символом *\n* и содержащая в формате JSON информацию.
Пример заголовка:

```json
{"currency":"USD","duration":3,"duration_uint":2,"sample_len":104,"symbols":["WLDAUD","WLDEUR","WLDGBP","WLDUSD","frxAUDCAD","frxAUDCHF","frxAUDJPY","frxAUDNZD","frxAUDUSD","frxEURAUD","frxEURCAD","frxEURCHF","frxEURGBP","frxEURJPY","frxEURNZD","frxEURUSD","frxGBPAUD","frxGBPCAD","frxGBPCHF","frxGBPJPY","frxGBPNZD","frxNZDUSD","frxUSDCAD","frxUSDJPY"]}
```

* *sample_len* - длина одного сэмпла соответствующего одной секунде дня, в байтах
* *currency* - валюта счета
* *duration* - экспирация опциона, зависит также от параметра *duration_uint*, в примере это 3 минуты
* *duration_uint* - единица измерения экспирации опциона, 2 соответствует минутам
* *symbols* - массив валютных пар

После заголовка идет массив бинарных данных, состоящий из сэмплов. Каждый сэмпл содержит значения процентов выплат для всех валютных пар в порядке их чередования, указанном в заголовке, а также содержит *timestamp* длиной 8 байт в конце сэмпла.
Для сокращения данных проценты выплат хранятся в виде 2 байт данных. Чтобы получить значение типа float или double, необходимо разделить считанное из файла значение на 1000. Пример:

```C++
unsigned short raw_proposal;
// читаем данные в raw_proposal
file.read(reinterpret_cast<char *>(&raw_proposal), sizeof (raw_proposal));

//...

// восстанавливаем данные
float proposal = (float)raw_proposal / 1000.0f;
```

Пример кода, который читает один сэмпл:

```C++
for(unsigned long s = 0; s < symbols_size; ++s) {
		unsigned short temp_buy = 0;
		unsigned short temp_sell = 0;
		// читаем сырые данные
		file.read(reinterpret_cast<char *>(&temp_buy), sizeof (temp_buy));
		file.read(reinterpret_cast<char *>(&temp_sell), sizeof (temp_sell));
		double buy_data = (double)temp_buy / 1000.0d; // процент выплат по ставка BUY
		double sell_data = (double)temp_sell / 1000.0d;

		//...
}
unsigned long long timestamp = 0;
file.read(reinterpret_cast<char *>(&timestamp), sizeof (timestamp));
```
Пример кода, который читает файл целиком:

```C++
	// загружаем настройки
	std::ifstream fin(file_name);
	std::string _s;
	std::getline(fin, _s);
	// парсим JSON строку
	json j_pp = json::parse(_s);
	// запоминаем смещение в файле (нужно так как бинарные данные расположены после заголовка)
	unsigned long start_pos = _s.size();
	fin.close();

	unsigned long sample_size = j_pp["sample_len"]; // длина одного сэмпла
	// читаем бинарные данные
	std::ifstream i(file_name, std::ios_base::binary);

	i.seekg (0, std::ios::end); // смещаемся в конец файла
	unsigned long data_size = i.tellg(); // получаем размер файла
	// 2- это магическое число появляется из-за символов переноса строки
	data_size = data_size - start_pos - 2; // получаем размер файла без заголовка
	i.seekg (start_pos + 2, std::ios::beg); // смещаемся в начало бинарных данных
	i.clear(); // очищаем флаги, навсякий случай

	// проверяем кратность размера данных размеру сэмпла
	if(data_size % sample_size != 0) {
		// если кратности нет, то дела плохи
		return false;
	}
	// получаем количество эсмплов
	unsigned long sample_num = data_size / sample_size;
	// получаем количество валютных пар
	unsigned long symbols_size = j_pp["symbols"].size();

	for(unsigned long n = 0; n < sample_num; ++n) { // читаем все сэмплы
		for(unsigned long s = 0; s < symbols_size; ++s) { // в каждом сэмпле читаем все валютные пары
			unsigned short temp_buy = 0;
			unsigned short temp_sell = 0;
			i.read(reinterpret_cast<char *>(&temp_buy), sizeof (temp_buy));
			i.read(reinterpret_cast<char *>(&temp_sell), sizeof (temp_sell));
			double buy_data = (double)temp_buy / 1000.0d;
			double sell_data = (double)temp_sell / 1000.0d;

			/* делаем что хотим с данными процентов выплат (buy_data, sell_data)
			 * данные представлены от 0.0 (0%) до 1.0 (100%), но иногда может быть и больше 100%
			 */
		}
		unsigned long long timestamp = 0;
		// читаем временную метку
		i.read(reinterpret_cast<char *>(&timestamp), sizeof (timestamp));
		// в переменной timestamp находится время, когда данные процентов выплат были актуальны
	}
	i.close();
	// конец
```

Бинарные файлы можно преобразовать в JSON файл при помощи конвертора (конвертор находится в архиве *bin.7z* в папке *bin*). 
Для преобразования необходимо через командную строку ввести следующую команду:

```
converter.exe <имя бинарного файла> to_json <имя файла json>
```

Для удобства можно скопировать все файлы из *bin.7z* в папку proposal_data и выполнять команды из нее.

Подробнее про JSON формат файла исторических данных процентов выплат. В начале файла есть строка-заголовок, содержащая важную информацию, пример:

```json
{"currency":"USD","duration":3,"duration_uint":2,"symbols":["WLDAUD","WLDEUR","WLDGBP","WLDUSD","frxAUDCAD","frxAUDCHF","frxAUDJPY","frxAUDNZD","frxAUDUSD","frxEURAUD","frxEURCAD","frxEURCHF","frxEURGBP","frxEURJPY","frxEURNZD","frxEURUSD","frxGBPAUD","frxGBPCAD","frxGBPCHF","frxGBPJPY","frxGBPNZD","frxNZDUSD","frxUSDCAD","frxUSDJPY"]}
```

Далее идут строки-сэмплы в формате JSON. Пример JSON строки:

```json
{"data":[{"buy":0.869,"sell":0.867,"symbol":"WLDAUD"},{"buy":0.846,"sell":0.825,"symbol":"WLDEUR"},{"buy":0.816,"sell":0.855,"symbol":"WLDGBP"},{"buy":0.835,"sell":0.835,"symbol":"WLDUSD"},{"buy":0.764,"sell":0.875,"symbol":"frxGBPNZD"},{"buy":0.869,"sell":0.769,"symbol":"frxNZDUSD"},{"buy":0.805,"sell":0.871,"symbol":"frxUSDCAD"},{"buy":0.869,"sell":0.869,"symbol":"frxUSDJPY"}],"time":1543553947}
```

* *time* - содержит timestamp, время GMT
* *data* - массив данных
* *buy* - процент выплат за сделку buy (PUT), от 0 до 1.0 (иногда может быть больше 1.0, 1.0 соответствует 100% выплате брокера)
* *sell* - процент выплат за сделку sell (CALL), от 0 до 1.0 (иногда может быть больше 1.0, 1.0 соответствует 100% выплате брокера)
* *symbol* - имя валютной пары (форекс валютные пары у брокера Binary начинаются с приставки *frx*)

### Полезные ссылки

C++ библиотека Websocket API брокера Binary.com [https://github.com/NewYaroslav/binary-cpp-api](https://github.com/NewYaroslav/binary-cpp-api)

Сайт брокера - [https://www.binary.com/](https://www.binary.com/)

С++ библиотека для парсинга JSON [https://github.com/nlohmann/json](https://github.com/nlohmann/json)

### Автор

elektro yar [electroyar2@gmail.com](electroyar2@gmail.com)
