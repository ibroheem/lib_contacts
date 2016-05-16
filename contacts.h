#ifndef CONTACTS
#define CONTACTS

#include "mib/io.h"
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include "misc.h"

using namespace std;
//using namespace mvc_utils::io;

namespace contacts
{
   namespace vcf
   {
      cchar* kwords[] = {"BEGIN","ADR","EMAIL", "END", "FN", "GEO", "IMPP", "KEY", "KIND", "LANG","LOGO", "N", "NOTE",
                         "ORG", "PHOTO", "REV", "SOURCE", "TEL", "TZ", "URL", "VERSION"
                        };

      class declaration;

      class attributes
      {
         public:
            attributes() = default;
            attributes(cchar* _attrs) : m_attr_string(_attrs) {}
            attributes(c_string& _attrs) : m_attr_string(_attrs) {}

            attributes(declaration&&);
            attributes(const declaration&);

            void build() { build(m_attr_string); }
            void build(c_string&);

            void build_from_decl(c_string&);
            bool exists();

            string& value(cchar* _name) { return m_attrs[_name]; }
            string& value(c_string& );
            vector<string> values();

         private:
            map<string, string> m_attrs = {};
            string m_attr_string;
      };

      void attributes::build(c_string& _attr_string)
      {
         vector<string> tmp = split(_attr_string, ';');
         for(string& s : tmp)
         {
            strip_char(s, ';');
            string stmp = split_return_last_of(s, '=');
            if (!stmp.empty())
               m_attrs.insert({split_return_first_of(s, '='), stmp});
         }

         //print_seq_cont(tmp, "\n");
      }

      string& attributes::value(c_string& _name)
      {
         return m_attrs[_name];
      }

      vector<string> attributes::values()
      {
         vector<string> ret;
         for_each(m_attrs.begin(), m_attrs.end(),
                  [&](pair<string, string> &&val)
         {
            ret.push_back(val.second);
         }
                 );

         return ret;
      }

      /** Denotes a vcf declaration line e.g:
          1. BEGIN:VCARD # decl
          2. VERSION:2.1 # decl
          3. N;CHARSET=UTF-8;ENCODING=8BIT:;Contact name
      **/
      class declaration
      {
         public:
            enum mode { flexible, strict };

            declaration() = default;
            declaration(cchar* _decl, mode _mode = mode::strict) : m_decl(_decl), m_mode(_mode) { }
            declaration(string& _decl, mode _mode = mode::strict): m_decl(_decl), m_mode(_mode) { }

            string& attribute_string();
            string& declaration_string() { return m_decl; }

            void set_declaration(cchar* _decl) { m_decl = _decl; }
            void set_declaration(c_string& _decl) { m_decl = _decl; }

            void set_mode(mode _mode) { m_mode = _mode; }

            string& keyword();
            string value();

            bool has_valid_keyword() { return has_valid_keyword(keyword()); }
            bool has_valid_keyword(c_string&);

            attributes& attribute();
            attributes& attribute(c_string&);

         private:
            string m_decl, m_keyword, m_attr_string;
            mode m_mode = mode::strict;
            attributes m_attribs;
      };

      using namespace contacts::vcf;

      string& declaration::attribute_string()
      {
         m_attr_string = split_return_first_of(m_decl, ':');
         m_attr_string = split_return_first_after(m_attr_string, ';');
         return m_attr_string;
      }

      bool declaration::has_valid_keyword(c_string& _target)
      {
         for(int i = 0; i < 20; ++i)
         {
            if (_target == kwords[i])
               return true;
         }
         return false;
      }

      string& declaration::keyword()
      {
         bool status = true;

         if (m_mode == mode::strict)
         {
            m_keyword = split_return_first_of(m_decl, ':');
            status = has_valid_keyword(m_keyword);
            if (status == false)
            {
               m_keyword = split_return_first_of(m_decl, ';');
               status = has_valid_keyword(m_keyword);
            }
         }

         return m_keyword;
      }

      string declaration::value()
      {
         return split_return_first_after(m_decl, ':');
      }

      attributes& declaration::attribute(c_string& attr_string)
      {
         m_attribs.build(attr_string);
         return m_attribs;
      }

      attributes& declaration::attribute()
      {
         return attribute(attribute_string());
      }

      class Contact
      {
         public:
            enum file_type { csv, vcf };
            enum contact_type { single, multiple };

            Contact() = default;

            void clear_all() { m_declarations.clear(); m_data.clear(); }

            //Suported: 2.1	3.0	4.0
            string address();
            string agent();
            string anniversary();
            string birthday();

            string calendar_();
            string calendar_uri();

            string categories();
            string contact_class();

            string& person_name();
            string person_name_formatted();
            string email();

            bool read_file(c_string&, file_type = vcf);

         private:
            //declaration m_declarations;
            vector<declaration> m_declarations;
            map<string, string> m_data;
      };

      string& Contact::person_name()
      {
         if (m_data.count("person_name") == 0)
         {
            for (declaration& decl : m_declarations)
            {
               if (decl.keyword() == "N")
               {
                  m_data["person_name"] = decl.value();
               }
            }
         }

         //return string("");
         return m_data["person_name"];
      }

      string Contact::person_name_formatted()
      {
         for (declaration& decl : m_declarations)
            if (decl.keyword() == "FN")
               return decl.value();

         return string();
      }

      bool Contact::read_file(c_string& fname, file_type ftype)
      {
         ifstream in(fname);

         switch(ftype)
         {
            case file_type::vcf :
            {
               if (in.is_open())
               {
                  declaration _decl;//(tmp, declaration::mode::strict);
                  string tmp;
                  while(!in.eof())
                  {
                     getline(in, tmp);
                     strip_line_breaks(tmp);
                     _decl.set_declaration(tmp);
                     if (_decl.has_valid_keyword())
                        m_declarations.push_back(_decl);
                     else
                        m_declarations.back().declaration_string().append(tmp);
                  }
                  in.close();
               }
               else
                  return false;

            } break;

            case file_type::csv :
            {
               //
            } break;

            default:
               in.close();
               break;
         }
         return true;
      }

   }

}

#endif
