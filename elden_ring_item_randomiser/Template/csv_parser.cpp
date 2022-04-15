
#include "csv_parser.hpp"
#include <fstream>

	namespace common {

		/*
		** csv_row
		*/

		csv_row::csv_row(const std::vector<std::string> &header)
			: m_header(header) {
		}

		csv_row::csv_row(const csv_row& other) :
			m_header(other.m_header), m_values(other.m_values)
		{
		}

		csv_row::~csv_row(void) {}

		unsigned int csv_row::size(void) const
		{
			return m_values.size();
		}

		void csv_row::set(const std::string &value)
		{
			m_values.push_back(value);
		}

		bool csv_row::set(const std::string &key, const std::string &value)
		{
			std::vector<std::string>::const_iterator it;
			int pos = 0;

			for (it = m_header.begin(); it != m_header.end(); ++it)
			{
				if (key == *it)
				{
					m_values[pos] = value;
					return true;
				}
				pos++;
			}
			return false;
		}

		const std::string csv_row::get(unsigned int value_position) const
		{
			if (value_position < m_values.size())
			{
				return m_values[value_position];
			}
			else
			{
				return std::string("can't return this value (doesn't exist)");
			}
		}

		const std::string csv_row::get(const std::string &key) const
		{
			std::vector<std::string>::const_iterator it;
			int pos = 0;

			for (it = m_header.begin(); it != m_header.end(); ++it)
			{
				if (key == *it)
					return m_values[pos];
				pos++;
			}
			return std::string("can't return this value (doesn't exist)");
		}
		/*
		** csv_parser
		*/
		csv_parser::csv_parser()
		{
			m_file_name = "";
			m_data_type = e_none;
			m_col_sep = ',';
			m_header.clear();
			_content.clear();
		}

		csv_parser::csv_parser(const csv_parser& other)
		{
			m_file_name = other.m_file_name;
			m_data_type = other.m_data_type;
			m_col_sep = other.m_col_sep;
			m_header = other.m_header;

			_content.clear();
			for (size_t i = 0; i < other._content.size(); ++i)
			{
				csv_row *new_row = new csv_row(*(other._content[i]));
				_content.push_back(new_row);
			}
		}

		csv_parser::~csv_parser(void)
		{
			std::vector<csv_row *>::iterator it;

			for (it = _content.begin(); it != _content.end(); ++it)
				delete *it;
		}

		void csv_parser::load(const std::string &data, const e_data_type &type /*= e_file*/)
		{
			if (e_none == type)
				return;

			std::string line;
			if (type == e_file)
			{
				m_file_name = data;
				m_data_type = e_file;
				std::ifstream ifile(m_file_name.c_str());

				if (ifile.is_open())
				{
					bool is_begin = true;
					while (ifile.good())
					{
						getline(ifile, line);
						if (is_begin)
						{
							is_begin = false;
							parse_header(line);
						}
						else
						{
							parse_content(line);
						}
					}
					ifile.close();
				}
			}
			else if (type == e_pure)
			{
				m_data_type = e_pure;
				bool is_begin = true;
				std::istringstream stream(data);
				while (std::getline(stream, line))
				{
					if (line != "")
					{
						if (is_begin)
						{
							is_begin = false;
							parse_header(line);
						}
						else
						{
							parse_content(line);
						}
					}
				}
			}
		}

		void csv_parser::parse_header(const std::string& new_value)
		{
			std::stringstream ss(new_value);
			std::string item;

			while (std::getline(ss, item, m_col_sep))
				m_header.push_back(item);
		}

		void csv_parser::parse_content(const std::string& new_value)
		{
			bool quoted = false;
			int token_start = 0;
			unsigned int i = 0;
			bool need_filter_quoted = false;

			csv_row *row = new csv_row(m_header);

			for (; i != new_value.length(); i++)
			{
				if (new_value.at(i) == '"')
				{
					quoted = ((quoted) ? (false) : (true));
					need_filter_quoted = true;
				}
				else if (new_value.at(i) == ',' && !quoted)
				{
					if (need_filter_quoted)
					{
						need_filter_quoted = false;
						std::string temp_value = new_value.substr(token_start + 1, i - token_start - 2);
						remove_double_quot_escape(temp_value);
						row->set(temp_value);
					}
					else
					{
						row->set(new_value.substr(token_start, i - token_start));
					}

					token_start = i + 1;
				}
			}
			if (need_filter_quoted)
			{
				need_filter_quoted = false;
				std::string temp_value = new_value.substr(token_start + 1, new_value.length() - token_start - 2);
				remove_double_quot_escape(temp_value);
				row->set(temp_value);
			}
			else
			{
				row->set(new_value.substr(token_start, new_value.length() - token_start));
			}
			_content.push_back(row);
		}

		csv_row* csv_parser::get_row(unsigned int row_position) const
		{
			if (row_position >= _content.size())
			{
				return nullptr;
			}

			return _content[row_position];
		}

		unsigned int csv_parser::get_row_count(void) const
		{
			return _content.size();
		}

		unsigned int csv_parser::get_column_count(void) const
		{
			return m_header.size();
		}

		std::vector<std::string> csv_parser::get_header(void) const
		{
			return m_header;
		}

		const std::string csv_parser::get_header_element(unsigned int pos) const
		{
// 			if (pos >= m_header.size())
// 				throw csv_error("can't return this header (doesn't exist)");
			return m_header[pos];
		}

		const std::string &csv_parser::get_file_name(void) const
		{
			return m_file_name;
		}

		void csv_parser::remove_double_quot_escape(std::string &str)
		{
			std::string str_src = "\"\"";
			std::string str_des = "\"";
			std::string::size_type pos = 0;
			std::string::size_type src_len = str_src.size();
			std::string::size_type des_len = str_des.size();
			pos = str.find(str_src, pos);
			while (pos != std::string::npos)
			{
				str.replace(pos, src_len, str_des);
				pos = str.find(str_src, pos + des_len);
			}
		}

		bool csv_parser::delete_row(unsigned int pos)
		{
			if (pos < _content.size())
			{
				delete *(_content.begin() + pos);
				_content.erase(_content.begin() + pos);
				return true;
			}

			return false;
		}

		bool csv_parser::add_row(unsigned int pos, const std::vector<std::string>& r)
		{
			csv_row* row = new csv_row(m_header);

			for (auto it = r.begin(); it != r.end(); ++it)
				row->set(*it);

			if (pos <= _content.size())
			{
				_content.insert(_content.begin() + pos, row);
				return true;
			}

			return false;
		}

		void csv_parser::add_double_quot_escape(std::string& str)
		{
			bool need_wrap = false;

			if (str.find(",") != std::string::npos || str.find("\"") != std::string::npos || str.find("\n") != std::string::npos)
				need_wrap = true;
			
			std::string str_src = "\"";
			std::string str_des = "\"\"";
			std::string::size_type pos = 0;
			std::string::size_type src_len = str_src.size();
			std::string::size_type des_len = str_des.size();
			pos = str.find(str_src, pos);
			while (pos != std::string::npos)
			{
				str.replace(pos, src_len, str_des);
				pos = str.find(str_src, pos + des_len);
			}

			if (need_wrap)
			{
				str = "\""+str;
				str = str + "\"";
			}
		}

		bool csv_parser::save_to_file(const std::string& file_name)
		{
			if (file_name.length()==0)
				return false;
			
			std::ofstream f;
			f.open(file_name.c_str(), std::ios::out | std::ios::trunc);

			//header
			unsigned int i = 0;
			for (auto it = m_header.begin(); it != m_header.end(); ++it)
			{
				std::string word = *it;
				add_double_quot_escape(word);
				f << word;
				if (i + 1 < m_header.size())
					f << ",";
				else
					f << std::endl;

				++i;
			}

			for (auto it = _content.begin(); it != _content.end(); ++it)
			{
				const csv_row& r_row = **it;

				for (int i_row = 0; i_row < r_row.get_colomn_num(); ++i_row)
				{
					std::string word = r_row.get(i_row);
					add_double_quot_escape(word);
					f << word;

					if (i_row < r_row.get_colomn_num() - 1)
					{
						f << ",";
					}
				}
				f << std::endl;
			}

			f.close();

			return true;
		}

		bool csv_parser::set_header_element(const unsigned int pos, const std::string& new_value)
		{
			if (m_header.size() > 0 && pos + 1 > m_header.size())
				return false;
			
			m_header[pos] = new_value;
			return true;
		}
	}

