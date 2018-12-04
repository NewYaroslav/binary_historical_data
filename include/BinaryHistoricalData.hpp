#ifndef BINARYHISTORICALDATA_HPP_INCLUDED
#define BINARYHISTORICALDATA_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <nlohmann/json.hpp>
#include <xtime.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
//------------------------------------------------------------------------------
class BinaryHistoricalData
{
public:
        using json = nlohmann::json;

        enum ErrorType {
                OK = 0,
                CAN_NOT_OPEN_FILE = -1,
                BAD_HEADER = -2,
                CORRUPTED_DATA = -3,
                MISSING_CURRENCY_PAIR = -4,
                FILE_NOT_UPLOADED = -5,
        };

        enum ContractType {
                BUY = 1,
                SELL = -1,
        };
private:
        std::unordered_map<std::string, int> map_proposal_symbol_;
        std::vector<std::vector<double>> proposal_buy_;
        std::vector<std::vector<double>> proposal_sell_;
        std::vector<unsigned long long> proposal_timestamp_;
        std::vector<std::string> proposal_symbols_;
        unsigned long long proposal_beg_timestamp_ = 0;
        unsigned long long proposal_end_timestamp_ = 0;
//------------------------------------------------------------------------------
        void clear_data()
        {
                map_proposal_symbol_.clear();
                proposal_buy_.clear();
                proposal_sell_.clear();
                proposal_timestamp_.clear();
                proposal_symbols_.clear();
                proposal_beg_timestamp_ = 0;
                proposal_end_timestamp_ = 0;
        }
//------------------------------------------------------------------------------
        int get_file_header(std::string file_name, json &j, unsigned long &header_pos)
        {
                std::ifstream file(file_name);
                if(!file)
                        return CAN_NOT_OPEN_FILE;
                std::string str_header;
                std::getline(file, str_header);
                const int LINE_TERMINATOR_LENGTH = 2;
                header_pos = str_header.size() + LINE_TERMINATOR_LENGTH;
                try {
                        j = json::parse(str_header);
                        return OK;
                }
                catch(...) {
                        return BAD_HEADER;
                }
        }
public:
//------------------------------------------------------------------------------
        /** \brief Инициализировать массив символов
         * \param symbols массив валютных пар
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int init_proposal_symbol(std::vector<std::string> &symbols)
        {
                if(symbols.size() == 0)
                       return CORRUPTED_DATA;
                clear_data();
                proposal_symbols_ = symbols;
                for(size_t s = 0; s < proposal_symbols_.size(); ++s) {
                        map_proposal_symbol_[proposal_symbols_[s]] = s;
                }
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Добавить проценты выплат к данным
         * timestamp должен постоянно возрастать
         * \param buy проценты выплат для сделок BUY
         * \param sell проценты выплат для сделок SELL
         * \param timestamp время
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int add_proposal(std::vector<double> &buy, std::vector<double> &sell, unsigned long long timestamp)
        {
                if(proposal_symbols_.size() == 0 ||
                        sell.size() != buy.size() ||
                        buy.size() == 0)
                        return CORRUPTED_DATA;
                if(proposal_timestamp_.size() > 0 &&
                        proposal_timestamp_.back() >= timestamp)
                        return CORRUPTED_DATA;
                proposal_timestamp_.push_back(timestamp);
                proposal_buy_.push_back(buy);
                proposal_sell_.push_back(sell);
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Добавить проценты выплат к данным
         * timestamp должен постоянно возрастать
         * \param buy проценты выплат для сделок BUY
         * \param sell проценты выплат для сделок SELL
         * \param timestamp время
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int set_proposal(std::vector<double> &buy, std::vector<double> &sell, unsigned long long timestamp)
        {
                if(proposal_symbols_.size() == 0 ||
                        sell.size() != buy.size() ||
                        buy.size() == 0)
                        return CORRUPTED_DATA;
                if(proposal_timestamp_.size() == 0)
                        return CORRUPTED_DATA;

                auto p = std::equal_range(proposal_timestamp_.begin(), proposal_timestamp_.end(), timestamp);
                if(*p.first == *p.second || p.first == proposal_timestamp_.end()) {
                        return CORRUPTED_DATA;
                }
                int indx = p.first - proposal_timestamp_.begin();

                proposal_timestamp_[indx] = timestamp;
                proposal_buy_[indx] = buy;
                proposal_sell_[indx] = sell;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Получить время начала данных процентов выплат
         * \param timestamp время
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int get_proposal_data_start_time(unsigned long long &timestamp)
        {
                if(proposal_beg_timestamp_ == 0)
                        return FILE_NOT_UPLOADED;
                timestamp = proposal_beg_timestamp_;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Получить время конца данных процентов выплат
         * \param timestamp время
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int get_proposal_data_stop_time(unsigned long long &timestamp)
        {
                if(proposal_end_timestamp_ == 0)
                        return FILE_NOT_UPLOADED;
                timestamp = proposal_end_timestamp_;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Получить массив валютных пар для данных по процентам выплат
         * \param symbols массив валютных пар
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int get_proposal_symbols(std::vector<std::string> &symbols)
        {
                if(proposal_symbols_.size() == 0)
                        return FILE_NOT_UPLOADED;
                symbols = proposal_symbols_;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Получить массив временных меток для данных по процентам выплат
         * \param timestamps массив временных меток
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        inline int get_proposal_timestamp(std::vector<unsigned long long> &timestamps)
        {
                if(proposal_timestamp_.size() == 0)
                        return FILE_NOT_UPLOADED;
                timestamps = proposal_timestamp_;
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Открыть бинарный файл с данными процентов выплат
         * \param file_name имя файла
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int open_proposal_data(std::string file_name)
        {
                clear_data();
                json j_header;
                unsigned long header_pos;
                int err = get_file_header(file_name, j_header, header_pos);
                if(err != OK)
                        return err;

                std::ifstream file(file_name, std::ios_base::binary);

                file.seekg (0, std::ios::end);
                const unsigned long data_size = file.tellg();
                const unsigned long samples_size = data_size - header_pos;
                file.seekg (header_pos, std::ios::beg);
                file.clear();

                if(!j_header["sample_len"].is_number())
                        return BAD_HEADER;
                const unsigned long sample_size = j_header["sample_len"];

                if(samples_size % sample_size != 0)
                        return CORRUPTED_DATA;
                const unsigned long num_samples = samples_size / sample_size;

                if(num_samples == 0)
                        return CORRUPTED_DATA;

                if(!j_header["symbols"].is_array())
                        return BAD_HEADER;

                proposal_symbols_.resize(j_header["symbols"].size());
                for(size_t i = 0; i < j_header["symbols"].size(); ++i) {
                        proposal_symbols_[i] = j_header["symbols"][i];
                }

                const unsigned long num_symbols = proposal_symbols_.size();
                for(unsigned long i = 0; i < num_symbols; ++i) {
                        map_proposal_symbol_[proposal_symbols_[i]] = i;
                } // for i

                proposal_timestamp_.resize(num_samples);
                proposal_buy_.resize(num_samples);
                proposal_sell_.resize(num_samples);
                for(unsigned long n = 0; n < num_samples; ++n) {
                        proposal_buy_[n].resize(num_symbols);
                        proposal_sell_[n].resize(num_symbols);
                        for(unsigned long s = 0; s < num_symbols; ++s) {
                                unsigned short temp_buy = 0;
                                unsigned short temp_sell = 0;
                                file.read(reinterpret_cast<char *>(&temp_buy), sizeof (temp_buy));
                                file.read(reinterpret_cast<char *>(&temp_sell), sizeof (temp_sell));
                                proposal_buy_[n][s] = (double)temp_buy / 1000.0d;
                                proposal_sell_[n][s] = (double)temp_sell / 1000.0d;
                        } // for s
                        file.read(reinterpret_cast<char *>(&proposal_timestamp_[n]), sizeof (unsigned long long));
                } // for n
                file.close();
                proposal_end_timestamp_ = proposal_timestamp_.back();
                proposal_beg_timestamp_ = proposal_timestamp_[0];
                return OK;
        }
//------------------------------------------------------------------------------
        BinaryHistoricalData()
        {

        }
//------------------------------------------------------------------------------
        /** \brief Получить проценты выплат
         * \param symbol валютная пара
         * \param timestamp время
         * \param proposal проценты выплат
         * \param contract_type тип контракта
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_proposal(std::string symbol,
                         unsigned long long timestamp,
                         int contract_type,
                         double &proposal)
        {
                if(proposal_timestamp_.size() == 0)
                        return FILE_NOT_UPLOADED;

                int indx_symbol;
                if(map_proposal_symbol_.find(symbol) == map_proposal_symbol_.end()) {
                        return MISSING_CURRENCY_PAIR;
                } else {
                        indx_symbol = map_proposal_symbol_[symbol];
                }
                auto p = std::equal_range(proposal_timestamp_.begin(), proposal_timestamp_.end(), timestamp);
                if(*p.first == *p.second || p.first == proposal_timestamp_.end()) {
                        return CORRUPTED_DATA;
                }
                int indx = p.first - proposal_timestamp_.begin();
                if(contract_type == BUY) {
                        proposal = proposal_buy_[indx][indx_symbol];
                } else
                if(contract_type == SELL) {
                        proposal = proposal_sell_[indx][indx_symbol];
                }
                return OK;
        }
//------------------------------------------------------------------------------
        /** \brief Получить массив процентов выплат
         * \param symbols валютные пары
         * \param timestamp время
         * \param buy массив процентов выплат для контракта buy
         * \param sell массив процентов выплат для контракта sell
         * \return состояние ошибки, вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_proposal_array(std::vector<std::string> &symbols,
                               unsigned long long timestamp,
                               std::vector<double> &buy,
                               std::vector<double> &sell)
        {
                if(proposal_timestamp_.size() == 0)
                        return FILE_NOT_UPLOADED;

                auto p = std::equal_range(proposal_timestamp_.begin(), proposal_timestamp_.end(), timestamp);
                if(*p.first == *p.second || p.first == proposal_timestamp_.end()) {
                        return CORRUPTED_DATA;
                }
                int indx = p.first - proposal_timestamp_.begin();

                if(symbols == proposal_symbols_) { // если список валютных пар совпадает
                        buy = proposal_buy_[indx];
                        sell = proposal_sell_[indx];
                } else { // если список валютных пар не совпадает
                        buy.resize(symbols.size());
                        sell.resize(symbols.size());
                        for(size_t s = 0; s < symbols.size(); ++s) {
                                int indx_symbol;
                                if(map_proposal_symbol_.find(symbols[s]) == map_proposal_symbol_.end()) {
                                        return MISSING_CURRENCY_PAIR;
                                } else {
                                        indx_symbol = map_proposal_symbol_[symbols[s]];
                                }
                                buy[s] = proposal_buy_[indx][indx_symbol];
                                sell[s] = proposal_sell_[indx][indx_symbol];
                        } // for s
                } // if
                return OK;
        }
//------------------------------------------------------------------------------
};

#endif // BINARYHISTORICALDATA_HPP_INCLUDED
