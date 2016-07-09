#if !defined TWITPP_UTILITY_QUERY_PARSER
#define      TWITPP_UTILITY_QUERY_PARSER

#include <unordered_map>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_grammar.hpp>

namespace twitpp {
namespace util {

namespace qi = boost::spirit::qi;

template <class String, class Iterator = typename String::const_iterator>
struct query_parser : qi::grammar<Iterator, std::unordered_map<String, String>()> {
  query_parser() : query_parser::base_type(map_) {
    map_ = pair_ % qi::lit('&');
    pair_ = string_ >> qi::lit('=') >> string_;
    string_ = +qi::char_("0-9a-zA-Z_-");
  }
  qi::rule<Iterator, std::unordered_map<String, String>()> map_;
  qi::rule<Iterator, std::pair<String, String>()> pair_;
  qi::rule<Iterator, String()> string_;
};

} // namespace util
} // namespace twitpp

#endif // TWITPP_UTILITY_QUERY_PARSER
