#pragma once
# include <stdexcept>
# include <string>
# include <vector>
# include <list>
# include <sstream>

typedef signed int          int32;

namespace common
{
	class csv_row
	{
	public:
		csv_row(const std::vector<std::string>&);
		~csv_row(void);
		csv_row(const csv_row& other);

	public:
		unsigned int size(void) const;
		void set(const std::string&);
		bool set(const std::string&, const std::string&);
		const std::string get(unsigned int) const;
		const std::string get(const std::string& key) const;

		size_t get_colomn_num() const
		{
			return m_values.size();
		}
	public:
		const std::vector<std::string> m_header;
		std::vector<std::string> m_values;
	};



	class csv_parser
	{
	public:
		enum e_data_type {
			e_none = 0,
			e_file,
			e_pure
		};

	public:
		csv_parser();
		~csv_parser(void);
		csv_parser(const csv_parser& other);

	public:
		void load(const std::string& data, const e_data_type& type = e_file);
		bool save_to_file(const std::string& file_name);
		const std::string& get_file_name(void) const;

		bool add_row(unsigned int pos, const std::vector<std::string>& r);
		bool delete_row(unsigned int pos);
		csv_row* get_row(unsigned int row) const;
		unsigned int get_row_count(void) const;
		unsigned int get_column_count(void) const;

		std::vector<std::string> get_header(void) const;
		const std::string get_header_element(unsigned int pos) const;
		bool set_header_element(const unsigned int pos, const std::string& new_value);
	protected:
		void parse_header(const std::string& new_value);
		void parse_content(const std::string& new_value);
		void remove_double_quot_escape(std::string& str);
		void add_double_quot_escape(std::string& str);

	private:
		std::string					m_file_name;
		e_data_type					m_data_type;
		char						m_col_sep;
		std::vector<std::string>	m_header;//first row of file
		std::vector<csv_row*>		_content;
	};
}
