#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>
#include "moses/Util.h"
#include "manual-label.h"
#include "DeEn.h"
#include "EnPhrasalVerb.h"

using namespace std;

bool g_debug = false;

Phrase Tokenize(const string &line);

int main(int argc, char** argv)
{
  cerr << "Starting" << endl;

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Print help messages")
    ("add", "additional options")
    ("source-language,s", po::value<string>()->required(), "Source Language")
    ("target-language,t", po::value<string>()->required(), "Target Language")
    ("revision,r", po::value<int>()->default_value(0), "Revision");

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc),
              vm); // can throw

    /** --help option
     */
    if ( vm.count("help")  )
    {
      std::cout << "Basic Command Line Parameter App" << std::endl
                << desc << std::endl;
      return EXIT_SUCCESS;
    }

    po::notify(vm); // throws on error, so do after help in case
                    // there are any problems
  }
  catch(po::error& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_FAILURE;
  }

  string sourceLang = vm["source-language"].as<string>();
  string targetLang = vm["target-language"].as<string>();
  int revision = vm["revision"].as<int>();

  cerr << sourceLang << " " << targetLang << " " << revision << endl;

  string line;
  size_t lineNum = 1;

  while (getline(cin, line)) {
    //cerr << lineNum << ":" << line << endl;
    if (lineNum % 1000 == 0) {
      cerr << lineNum << " ";
    }

    Phrase source = Tokenize(line);

    if (sourceLang == "de" && targetLang == "en") {
    	LabelDeEn(source, cout);
    }
    else if (sourceLang == "en") {
    	EnPhrasalVerb(source, revision, cout);
    }

    ++lineNum;
  }



  cerr << "Finished" << endl;
  return EXIT_SUCCESS;
}

Phrase Tokenize(const string &line)
{
  Phrase ret;

  vector<string> toks = Moses::Tokenize(line);
  for (size_t i = 0; i < toks.size(); ++i) {
    Word word = Moses::Tokenize(toks[i], "|");
    ret.push_back(word);
  }

  return ret;
}

bool IsA(const Phrase &source, int pos, int offset, int factor, const string &str)
{
  pos += offset;
  if (pos >= source.size() || pos < 0) {
    return false;
  }

  const string &word = source[pos][factor];
  vector<string> soughts = Moses::Tokenize(str, " ");
  for (int i = 0; i < soughts.size(); ++i) {
    string &sought = soughts[i];
    bool found = (word == sought);
    if (found) {
      return true;
    }
  }
  return false;
}


void OutputWithLabels(const Phrase &source, const Ranges ranges, ostream &out)
{
  // output sentence, with labels
  for (int pos = 0; pos < source.size(); ++pos) {
	// output beginning of label
	for (Ranges::const_iterator iter = ranges.begin(); iter != ranges.end(); ++iter) {
	  const Range &range = *iter;
	  if (range.first == pos) {
		out << "<tree label=\"reorder-label\"> ";
	  }
	}

	const Word &word = source[pos];
	out << word[0] << " ";

	for (Ranges::const_iterator iter = ranges.begin(); iter != ranges.end(); ++iter) {
	  const Range &range = *iter;
	  if (range.second == pos) {
		out << "</tree> ";
	  }
	}
  }
  out << endl;

}