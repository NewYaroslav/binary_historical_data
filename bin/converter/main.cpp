#include <iostream>
#include <fstream>
#include <dir.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdarg>

using namespace std;
using json = nlohmann::json;

bool open_json(std::string file_name, std::string out_file_name);
bool open_bin(std::string file_name, std::string out_file_name);
void write_binary_file(std::string file_name,
                       std::vector<std::string> &symbols,
                       int duration,
                       int duration_uint,
                       std::string currency,
                       std::vector<double> &buy_data,
                       std::vector<double> &sell_data,
                       unsigned long long timestamp);
std::string format(const char *fmt, ...);

int main(int argc, char *argv[])
{
        if(argc == 4) {
                std::string in_file_name = std::string(argv[1]);
                std::string out_file_name = std::string(argv[3]);
                std::string cmd_type = std::string(argv[2]);
                if(cmd_type == "to_bin") {
                        if(!open_json(in_file_name, out_file_name)) {
                                std::cout << "input or output file error!" << std::endl;
                                std::cin;
                        }
                        std::cout << "ok" << std::endl;
                } else
                if(cmd_type == "to_json") {
                        if(!open_bin(in_file_name, out_file_name)) {
                                std::cout << "input or output file error!" << std::endl;
                                std::cin;
                        }
                        std::cout << "ok" << std::endl;
                } else {
                        std::cout << "cmd error!" << std::endl;
                }

        } else {
                std::cout << "parameter error! argc: " << argc << std::endl;
                for(int i = 0; i < argc; ++i) {
                        std::cout << std::string(argv[i]) << std::endl;
                }
                std::cin;
        }
        return 0;
}

bool open_json(std::string file_name, std::string out_file_name) {
        std::ifstream i(file_name);
        if(!i)
                return false;
        std::string s;
        int indx = 0;
        std::getline(i, s);
        json j_pp = json::parse(s);
        std::vector<string> symbols = j_pp["symbols"];
        int duration = j_pp["duration"];
        int duration_uint = j_pp["duration_uint"];
        std::string currency = j_pp["currency"];

        std::cout << "duration: " << duration << std::endl;
        std::cout << "duration_uint: " << duration_uint << std::endl;
        std::cout << "currency: " << currency << std::endl;
        std::cout << "symbols: " << std::endl;
        for(size_t s = 0; s < symbols.size(); ++s) {
                std::cout << symbols[s] << std::endl;
        }

        while(std::getline(i, s)) {
                std::cout << "indx " << indx << "\r";
                json j = json::parse(s);
                std::vector<double> buy_data(j["data"].size());
                std::vector<double> sell_data(j["data"].size());
                for(size_t i = 0; i < j["data"].size(); ++i) {
                        buy_data[i] = j["data"][i]["buy"];
                        sell_data[i] = j["data"][i]["sell"];
                }
                write_binary_file(out_file_name,
                                  symbols,
                                  duration,
                                  duration_uint,
                                  currency,
                                  buy_data,
                                  sell_data,
                                  j["time"]);
                indx++;
        }
        std::cout << "indx " << indx << std::endl;
        i.close();
        return true;
}

void write_binary_file(std::string file_name,
                       std::vector<std::string> &symbols,
                       int duration,
                       int duration_uint,
                       std::string currency,
                       std::vector<double> &buy_data,
                       std::vector<double> &sell_data,
                       unsigned long long timestamp)
{
        // проверяем, был ли создан файл?
        std::ifstream fin(file_name);
        if(!fin) {
                // сохраняем заголовок файла
                std::ofstream fout(file_name);
                json j;
                j["symbols"] = symbols;
                j["duration"] = duration;
                j["duration_uint"] = duration_uint;
                j["currency"] = currency;
                const int _sample_len = (2 * sizeof(unsigned short)) * symbols.size() + sizeof (unsigned long long);
                j["sample_len"] = _sample_len;
                fout << j.dump() << "\n";
                fout.close();
                fin.close();
        } else {
                fin.close();
        }
        // сохраняем
        std::ofstream fout(file_name, std::ios_base::binary | std::ios::app);
        for(int i = 0; i < buy_data.size(); i++) {
                unsigned short temp_buy = buy_data[i] * 1000;
                unsigned short temp_sell = sell_data[i] * 1000;
                fout.write(reinterpret_cast<char *>(&temp_buy),sizeof (temp_buy));
                fout.write(reinterpret_cast<char *>(&temp_sell),sizeof (temp_sell));
        }
        fout.write(reinterpret_cast<char *>(&timestamp),sizeof (timestamp));
        fout.close();
}

bool open_bin(std::string file_name, std::string out_file_name) {
        // загружаем настройки
        std::ifstream fin(file_name);
        std::string _s;
        std::getline(fin, _s);
        json j_pp = json::parse(_s);
        unsigned long start_pos = _s.size();
        fin.close();

        unsigned long sample_size = j_pp["sample_len"];
        // читаем и сохраняем
        std::ifstream i(file_name, std::ios_base::binary);
        if(!i)
                return false;
        i.seekg (0, std::ios::end);
        unsigned long data_size = i.tellg();
        data_size = data_size - start_pos - 2;
        i.seekg (start_pos + 2, std::ios::beg);
        i.clear();

        if(data_size % sample_size != 0) {
                std::cout << "data_size % sample_size != 0" << std::endl;
                return false;
        }

        unsigned long sample_num = data_size / sample_size;
        unsigned long symbols_size = j_pp["symbols"].size();

        json j_header;
        j_header["symbols"] = j_pp["symbols"];
        j_header["duration"] = j_pp["duration"];
        j_header["duration_uint"] = j_pp["duration_uint"];
        j_header["currency"] = j_pp["currency"];

        std::cout << "sample_num: " << sample_num << std::endl;
        std::cout << "sample_size: " << sample_size << std::endl;
        std::cout << "symbols: " << symbols_size << std::endl;

        std::ofstream o(out_file_name);
        if(!o)
                return false;
        o << j_header.dump() << "\n";

        for(unsigned long n = 0; n < sample_num; ++n) {
                std::vector<double> buy_data(symbols_size);
                std::vector<double> sell_data(symbols_size);
                json j;
                for(unsigned long s = 0; s < symbols_size; ++s) {
                        unsigned short temp_buy = 0;
                        unsigned short temp_sell = 0;
                        i.read(reinterpret_cast<char *>(&temp_buy), sizeof (temp_buy));
                        i.read(reinterpret_cast<char *>(&temp_sell), sizeof (temp_sell));
                        double buy_data = (double)temp_buy / 1000.0d;
                        double sell_data = (double)temp_sell / 1000.0d;
                        j["data"][s]["buy"] = format("%.3f", buy_data);
                        j["data"][s]["sell"] = format("%.3f", sell_data);
                        j["data"][s]["symbol"] = j_pp["symbols"][s];
                        //std::cout << "buy: " << buy_data << std::endl;
                }
                unsigned long long timestamp = 0;
                i.read(reinterpret_cast<char *>(&timestamp), sizeof (timestamp));
                j["time"] = timestamp;
                //std::cout << "time: " << timestamp << std::endl;
                o << j.dump() << "\n";
                std::cout << format("progress: %.1f %%", 100.0d * ((double)n/ (double)sample_num)) << "\r";
        }
        std::cout << "progress: 100.0 %%\n";
        i.close();
        o.close();
        return true;
}

std::string format(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        std::vector<char> v(1024);
        while (true)
        {
                va_list args2;
                va_copy(args2, args);
                int res = vsnprintf(v.data(), v.size(), fmt, args2);
                if ((res >= 0) && (res < static_cast<int>(v.size())))
                {
                    va_end(args);
                    va_end(args2);
                    return std::string(v.data());
                }
                size_t size;
                if (res < 0)
                    size = v.size() * 2;
                else
                    size = static_cast<size_t>(res) + 1;
                v.clear();
                v.resize(size);
                va_end(args2);
        }
}
