#ifndef DATA_WRAPPER_H_
#define DATA_WRAPPER_H_

#include <string>
#include <typeinfo>
#include <stdint.h>

namespace dw {

	class data_wrapper
	{
	protected:
		class placeholder
		{
		public:
			placeholder() { }
			virtual ~placeholder() { }

			virtual const std::type_info & type_info() const = 0;
			virtual placeholder* clone() const = 0;
		};

		template< typename T >
		class data_holder : public placeholder
		{
		public:
			T value;
		public:
			data_holder(const T& val) : value(val)
			{

			}
			virtual ~data_holder()
			{

			}
			virtual const std::type_info & type_info() const
			{
				return typeid(T);
			}
			virtual placeholder* clone() const
			{
				return new data_holder(value);
			}
		};

	protected:
		placeholder* content;
		//const char* _content;
	public:
		data_wrapper() : content(NULL)
		{

		}
		template< typename T >
		data_wrapper(const T& val) : content(new data_holder< T >(val))
		{

		}
		data_wrapper(const char* val) : content(NULL)
		{
            if (val != NULL)
            {
                content = new data_holder< std::string >(std::string(val));
            }
		}
		data_wrapper(const data_wrapper& rhs) : content(rhs.content != NULL ? rhs.content->clone() : NULL)
		{

		}
		~data_wrapper()
		{
			delete content;
			content = NULL;
		}

		// Basic operation

		const std::type_info& type_info() const
		{
			return content != NULL ? content->type_info() : typeid(void);
		}

		data_wrapper& swap(data_wrapper &rhs)
		{
			std::swap(content, rhs.content);
			return (*this);
		}

		data_wrapper& operator=(const data_wrapper &rhs)
		{
			return swap(data_wrapper(rhs));
		}

		bool valid() const
		{
			return ((this != NULL) && (content != NULL));
		}

		// Template operation

		template< typename T >
		bool istype()
		{
			data_holder< T >* holder = dynamic_cast<data_holder< T >*>(content);
			return (holder != NULL);
		}

		template< typename T >
		bool set(T t)
		{
			data_holder< T >* holder = dynamic_cast<data_holder< T >*>(content);
			if (holder != NULL) { holder->set(t); return true;  }
			else { return false;  }
		}
		bool set(const data_wrapper& t)
		{
			swap(data_wrapper(t));
			return true;
		}
        template< typename T >
        bool check(const T& expect)
        {
            return ((type_info() == typeid(T) && value_as< T >() == expect));
        }
        bool check(const char* expect)
        {
            bool ret = false;
            if (expect != NULL)
            {
                std::string val = std::string(expect);
                ret = check(val);
            }
            else
            {
                ret = (!valid());
            }
            return ret;
        }

		template< typename T >
		T* pointer_as()
		{
			//if (typeid(T) == typeid(const char*))
			//{
			//	std::string* val = pointer_as< std::string >();
			//	_content = (val != NULL ? val->c_str() : NULL);
			//	return (T*)&_content;
			//}

			//if (content != NULL)
			//{
			//	const char* content_type = content->type_info().name();
			//	const char* function_type = typeid(T).name();
			//	(void)(content_type, function_type, "For debug.");
			//}

			return ((content != NULL) && (content->type_info() == typeid(T))) ?
				&(static_cast<data_holder< T >*>(content))->value : NULL;
		}

		template< typename T >
		T value_as()
		{
			return value_as(T());
		}

		template< typename T >
		T value_as(T default_val)
		{
			T* val = pointer_as< T >();
			return (val != NULL ? (*val) : default_val);
		}
	};

	typedef data_wrapper any;

}

#endif // DATA_WRAPPER_H_