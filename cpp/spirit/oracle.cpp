/*=============================================================================
    Copyright (c) 2002-2010 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace client
{
    enum DayOfWeek
    {
        Mon, Tue, Wed, Thu, Fri, Sat, Sun
    };
    
    enum MonthOfYear
    {
        Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
    };

    struct date
    {
        DayOfWeek day_of_week;
        MonthOfYear month_of_year;
        unsigned day_of_month;
        unsigned hour;
        unsigned minute;
        unsigned second;
        int year;
    };
    
    struct tagvalue
    {
        std::string tag;
        std::string value;
    };
    
    struct event
    {
        date d;
        std::vector<tagvalue> tvs;
    };
}

// We need to tell fusion about our tagvalue struct
// to make it a first-class fusion citizen. This has to
// be in global scope.

BOOST_FUSION_ADAPT_STRUCT(
    client::date,
    (client::DayOfWeek, day_of_week)
    (client::MonthOfYear, month_of_year)
    (unsigned, day_of_month)
    (unsigned, hour)
    (unsigned, minute)
    (unsigned, second)
    (int, year)
)

BOOST_FUSION_ADAPT_STRUCT(
    client::tagvalue,
    (std::string, tag)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    client::event,
    (client::date, d)
    (std::vector<client::tagvalue>, tvs)
)

namespace client
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    struct EventTags : qi::symbols<char, std::string>
    {
        EventTags()
        {
            add
                ("ACTION", std::string("ACTION"))
                ("DATABASE USER", std::string("DATABASE USER"))
                ("PRIVILEGE", std::string("PRIVILEGE"))
                ("CLIENT USER", std::string("CLIENT USER"))
                ("CLIENT TERMINAL", std::string("CLIENT TERMINAL"))
                ("STATUS", std::string("STATUS"))                
            ;
        }

    };
    
    struct DayOfWeekTags : qi::symbols<char, DayOfWeek>
    {
        DayOfWeekTags()
        {
            add
                ("Mon", Mon)
                ("Tue", Tue)
                ("Wed", Wed)
                ("Thu", Thu)
                ("Fri", Fri)
                ("Sat", Sat)
                ("Sat", Sun)
            ;
        }

    };
    
    struct MonthOfYearTags : qi::symbols<char, MonthOfYear>
    {
        MonthOfYearTags()
        {
            add
                ("Jan", Jan)
                ("Feb", Feb)
                ("Mar", Mar)
                ("Apr", Apr)
                ("May", May)
                ("Jun", Jun)
                ("Jul", Jul)
                ("Aug", Aug)
                ("Sep", Sep)
                ("Oct", Oct)
                ("Nov", Nov)
                ("Dec", Dec)
            ;
        }

    };
    
    
    template <typename Iterator>
    struct date_parser : qi::grammar<Iterator, date(), ascii::space_type>
    {
        date_parser() : date_parser::base_type(start)
        {
            start %= day_of_week >> month_of_year >> qi::ushort_
                >> qi::ushort_ >> ':' >> qi::ushort_ >> ':' >> qi::ushort_
                >> qi::int_
                ;
        }
        
        DayOfWeekTags       day_of_week;
        MonthOfYearTags     month_of_year;
        qi::rule<Iterator, date(), ascii::space_type> start;
    };
    
    ///
    /// This parser parses name-value pairs of the form <tag> ':' <value>
    /// where <value> is a string wrapped in single quotes or a sequence
    ///
    template <typename Iterator>
    struct tagvalue_parser : qi::grammar<Iterator, tagvalue(), ascii::space_type>
    {
        tagvalue_parser() : tagvalue_parser::base_type(start)
        {
            string %= qi::lexeme[+(ascii::char_ - qi::eol)];
            quoted_string %= qi::lexeme['\'' >> +(ascii::char_ - '\'') >> '\''];
            
            start %= tag >> ':' >> (quoted_string | string);
        }
        
        EventTags tag;
        qi::rule<Iterator, std::string()> string;
        qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
        qi::rule<Iterator, tagvalue(), ascii::space_type> start;
    };
    
    
    ///
    /// This parser parsed events, return event structures
    ///
    template <typename Iterator>
    struct event_parser : qi::grammar<Iterator, event(), ascii::space_type>
    {
        event_parser() : event_parser::base_type(start)
        {
            tvs %= *(tv_parser) ;
            start %= date >> tvs;
        }

        date_parser<Iterator> date;
        tagvalue_parser<Iterator> tv_parser;
        qi::rule<Iterator, std::vector<tagvalue>(), ascii::space_type> tvs;
        qi::rule<Iterator, event(), ascii::space_type> start;
    };
    

}

int
main()
{
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tAn tagvalue parser for Spirit...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";

    std::cout
        << "Give me an tagvalue of the form :"
        << "<tag> : '<value>' \n";
    std::cout << "Type [q or Q] to quit\n\n";

    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    // typedef client::tagvalue_parser<iterator_type> tagvalue_parser;
    typedef client::event_parser<iterator_type> event_parser;

    // tagvalue_parser g; // Our grammar
    event_parser g; // Our grammar
    std::string accum;
    std::string str;

    while (getline(std::cin, str))
    {
        if (str.empty())
            break;
        
        if (str == ".")
        {
            // client::tagvalue result;
            client::event result;
            // std::cout << "IN: " << accum << std::endl;
            std::string::const_iterator iter = accum.begin();
            std::string::const_iterator end = accum.end();
            bool r = phrase_parse(iter, end, g, space, result);
    
            if (r && iter == end)
            {
                std::cout << boost::fusion::tuple_open('[');
                std::cout << boost::fusion::tuple_close(']');
                std::cout << boost::fusion::tuple_delimiter(", ");
    
                std::cout << "-------------------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << "year: " << result.d.year << std::endl;
                std::cout << "month_of_year: " << result.d.month_of_year << std::endl;
                std::cout << "day_of_month: " << result.d.day_of_month << std::endl;
                std::cout << "hour: " << result.d.hour << std::endl;
                std::cout << "minute: " << result.d.minute << std::endl;
                std::cout << "second: " << result.d.second << std::endl;
                for (std::vector<client::tagvalue>::iterator pos = result.tvs.begin();  pos != result.tvs.end();  ++pos)
                {
                    std::cout << "result.tag:   \"" << pos->tag << '"' << std::endl;
                    std::cout << "result.value: \"" << pos->value << '"' << std::endl;
                }
                std::cout << "\n-------------------------\n";
            }
            else
            {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed\n";
                std::cout << "-------------------------\n";
                std::cout << "UP TO: " << std::string(iter, end) << std::endl;
            }
            accum = "";
        }
        else
        {
            accum += str + '\n'; // '\n';
        }

    }
    std::cout << "Bye... :-) \n\n";
    return 0;
}
