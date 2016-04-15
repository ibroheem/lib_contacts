#ifndef HEADER_DC7527D187C1340
#define HEADER_DC7527D187C1340

#include <pugi/pugixml.hpp>
#include <string>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

#include <QMessageBox>

#include <mib/io.h>
#include <mib/string_operations.h>
#include <mib/pugi/operations.h>

using namespace mvc_utils::io;
using namespace mvc_utils::string_operations;
using namespace pugi;
using namespace std;

using namespace pugi_operations::write;

namespace conversion
{
#ifdef SOCI_H_INCLUDED
  #include <soci/soci.h>
  #include <soci/soci-sqlite3.h>
  class sqlite_to_xml_soci
  {
    public:
      sqlite_to_xml_soci() = default;
  };

#endif // SOCI_H_INCLUDED


//#ifndef QSQLDATABASE_H

  class sqlite_to_xml_Qt
  {
    public:
      sqlite_to_xml_Qt()  {  }
      sqlite_to_xml_Qt (cchar *);
      sqlite_to_xml_Qt (cchar *, cchar *);

      void set_db_name (const char *_name) {  m_db_name = _name; };
      void set_db_name (const QString &_name) {  set_db_name (_name); };

      void set_output_name (const char *_name) {  m_xml_name = _name; };
      void set_output_name (const QString &_name) {  set_output_name (_name); };

      void convert();

      void convert_n_write() { convert_n_write(m_xml_name); };
      void convert_n_write (cchar * _out_name) { convert(); write_file(_out_name); }

      /** clears the xml internal document content(the output file's nor database's)  */
      void reset_xml_doc() { doc.reset(); }

      /** clears the xml internal document content(the output file's nor database's) and creates a child in which
       other children might be added  */
      void reset_xml_doc_w_child(cchar *_header) { doc.reset(); header = doc.append_child (_header); }

      /** writes/saves the result of the conversion prcc to _output_name, conversion must've been done */
      void write_file() { write_file (m_xml_name); };
      void write_file (cchar* _output_name) { doc.save_file (_output_name); };

    private:
      bool open_database_conn() { return m_database.open();}
      void set_database_name() { m_database.setDatabaseName (m_db_name); };

      cchar* m_db_name = "", * m_xml_name = "output.xml";//file_name_wo_ext(string(m_db_name)).append(".xml").c_str();
      QSqlDatabase m_database = QSqlDatabase::addDatabase ("QSQLITE");

      QStringList m_tables;

      xml_document doc;
      xml_node header = doc.append_child ("header");

  };

  /** Ctor taking only database name, output.xml is the name of the generated output*/
  sqlite_to_xml_Qt::sqlite_to_xml_Qt (cchar * _db_name) : m_db_name (_db_name)
  {

  }

  /** _db_name is the database name, _output_name is the name of the generated output*/
  sqlite_to_xml_Qt::sqlite_to_xml_Qt (cchar* _db_name, cchar * _output_name) : m_db_name (_db_name),
                                                                               m_xml_name (_output_name)
  {

  }

  /** Does the conversion sets db name, opens it, get tables, creates query, then db is read for conversion to xml.
      Does not do the saving to file
   */
  void sqlite_to_xml_Qt::convert ()
  {
    set_database_name();
    open_database_conn();
    m_tables = m_database.tables();

    for (int i = 0; i < m_tables.size(); ++i)
    {
      QSqlQuery query ("SELECT * FROM " + m_tables[i], m_database);

      int l = 1, rec_len = query.record().count();

      append_attribute(header, "database_name" , m_db_name);
      xml_node table = append_child (header, "table");
      append_attribute(table, "name", m_tables[i]);

      while (query.next())
      {
        xml_node row = append_child (table, "row");
        row.append_attribute("row_id").set_value(l);

        for (int j = 0; j < rec_len; ++j)
          append_attribute (row, query.record().fieldName (j), query.value (j).toString() );
        ++l;
      }
    }
  }

  void usage()
  {
    {
    /*sqlite_to_xml_Qt proj("/home/ibroheem/usr/devs/sql/Book2.db", "Book1.xml");
    proj.convert_n_write();
    proj.reset_xml_doc_w_child("header"); //If not called, new conversion will be appended

    proj.set_db_name("/home/ibroheem/usr/devs/sql/Book10.db");
    proj.set_output_name("Book10.xml");
    proj.convert_n_write();*/
    }
  }

//#endif // QSQLDATABASE_H
}

#endif // header guard
