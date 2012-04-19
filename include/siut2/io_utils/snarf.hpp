#ifndef _SIUT_IO_SNARF_HPP
#define _SIUT_IO_SNARF_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#ifdef DEBUG_IO
#define THROW_IO_ERROR
#endif

namespace siut2
{
  /** Namespace that contains tools for reading files, dumping files to ostream with line numbers, and adding line numbers to strings.*/
  namespace io_utils
  {

    /** Reads an entire file into a string and returns it.
     * If DEBUG_IO is defined, will throw a runtime_error if there is a problem opening the file.
     */
    inline std::string snarfFile(const std::string &fname)
    {
      std::ifstream file( fname.c_str());
#ifdef THROW_IO_ERROR
      if (!file)
	{
	  std::stringstream s;
	  s << "error opening " << fname << " in " << __FILE__ << " at " << __LINE__ << std::endl;
	  throw std::runtime_error(s.str());
	}
#endif
      return std::string( std::istreambuf_iterator<char>(file),
		     std::istreambuf_iterator<char>());
    }


      /*!
       *Simple* preprocessing of multi-line string: #include "filename" is replaced with contents of "filename". 
       Works for nested includes, but loops will not be detected. 
       The string '#include "' must be at the start of a line.
       Also, 'filename' cannot contain '"'.
       Using snarfFile to get "included" files for insertion, and for error handling.
      */
      inline void preprocessSnarfedFile(std::string &src)
      {
	  const std::string inc_str = "\n#include \"";
	  size_t cur_pos = 0;

	  while (cur_pos!=std::string::npos) {
	      const size_t pos = src.find(inc_str, cur_pos);
	      if (pos!=std::string::npos) {
		  const size_t pos2 = src.find("\"", pos+inc_str.length());
		  if (pos2!=std::string::npos) {
		      const std::string fname = src.substr(pos+inc_str.length(), pos2-pos-inc_str.length());
		      const std::string src2 = snarfFile(fname);
		      src.replace(pos, pos2-pos+1, src2);
		      cur_pos = pos;
		  } else {
#ifdef THROW_IO_ERROR
		      std::stringstream s;
		      s << "Hmm. No terminating double-quote?! Error opening include file " << fname << " in " << __FILE__ << " at " << __LINE__ << std::endl;
		      throw std::runtime_error(s.str());
#endif
		  }
	      } else
		  cur_pos=std::string::npos;
	  }
      }
	


    /** Copies the string, adding line numbers, and returns the copied string. */
    inline std::string addLineNumbers(const std::string &orig)
    {
      std::stringstream input(orig), ret;
      std::string temp;
      size_t lCounter = 1;

      while(getline(input, temp))
	{
	  ret << lCounter++ << ": " + temp << std::endl;
	}
      return ret.str();

    }

    /** Dumps the string to the specified ostream, adding linenumbers on the way.
     * \param orig the string to process.
     * \param output the ostream to dump the file to.
     */
    inline void dumpSourceWithLineNumbers(const std::string &orig, std::ostream &output = std::cerr)
    {
      std::stringstream input(orig);
      std::string temp;
      size_t lCounter = 1;
      while(getline(input, temp))
	{
	  output << std::setw(3) << lCounter++ << ": ";
	  output << temp << std::endl;
	}
      output.flush();
    }

  }//end IO namespace
}//end siut namespace
#endif
