#ifndef CONTACTS
#define CONTACTS

#include "mib/io.h"
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include "misc.h"

using namespace std;
using namespace mvc_utils::io;

namespace contacts
{
   namespace vcf
   {
      cchar* kwords[] = {"BEGIN","ADR","EMAIL", "END", "FN", "GEO", "IMPP", "KEY", "KIND", "LANG","LOGO", "NOTE",
                         "ORG", "PHOTO", "REV", "SOURCE", "TEL", "TZ", "URL"};

      class attributes;

      /** Denotes a vcf declaration line e.g:
          1. BEGIN:VCARD
          2. VERSION:2.1
          3. N;CHARSET=UTF-8;ENCODING=8BIT:;Contact name
      **/
      class declaration
      {
         public:
            enum class mode { flexible, strict };

            declaration() = default;
            declaration(cchar* _decl, mode _mode = mode::strict) : m_decl(_decl), m_mode(_mode) { }
            declaration(string& _decl, mode _mode = mode::strict): m_decl(_decl), m_mode(_mode) { }

            string attribute_string();

            void set_declaration(cchar* _decl) { m_decl = _decl; }
            void set_declaration(c_string& _decl) { m_decl = _decl; }

            void set_mode(mode _mode) { m_mode = _mode; }

            string keyword();
            string value();

            bool is_valid_keyword() { return is_valid_keyword(split_return_first_of(m_decl, ';')); }
            bool is_valid_keyword(c_string&);

            attributes attribute();
            attributes attribute(c_string&);

         private:
            string m_decl, m_keyword;
            mode m_mode = mode::strict;
      };

      using namespace contacts::vcf;

      string declaration::attribute_string()
      {
         return split_return_first_after(split_return_first_of(m_decl, ':'), ';');
      }

      bool declaration::is_valid_keyword(c_string& _target)
      {
         for(int i = 0; i < 6; ++i)
         {
            if (_target == kwords[i])
               return true;
         }
         return false;
      }

      string declaration::keyword()
      {
         string ret;
         bool status = true;

         if (m_mode == mode::strict)
         {
            ret = split(m_decl, ';').front();
            status = is_valid_keyword(ret);
         }
         else
            ret = split(m_decl, ';').front();

         return (status== true) ? ret : "";
      }

      string declaration::value()
      {
         return split_return_first_after(m_decl, ':');
      }

      class attributes
      {
         public:
            attributes(cchar* _attrs) : m_attr_string(_attrs) {}
            attributes(c_string& _attrs) : m_attr_string(_attrs) {}

            attributes(declaration&&);
            attributes(const declaration&);

            void build() { build(m_attr_string); }
            void build(c_string&);

            void build_from_decl(c_string&);
            bool exists();

            string value(cchar* _name) { return m_attrs[_name]; }
            string value(c_string& );
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

      string attributes::value(c_string& _name)
      {
         return m_attrs[_name];
      }

      attributes declaration::attribute(c_string& attr_string)
      {
         attributes attribs(attr_string);
         attribs.build();
         return attribs;
      }

      attributes declaration::attribute()
      {
         return attribute(attribute_string());
      }

   }

}

#endif
