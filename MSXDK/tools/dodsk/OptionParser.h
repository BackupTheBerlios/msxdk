#ifndef _OPTIONPARSER_H_
#define _OPTIONPARSER_H_

class OptionParser {
private:
	int		   m_argc;
	char	** m_argv;	
	int		   m_argi;
	int		   m_chari;
	int		   m_option;
	char	 * m_optarg;
	int		   m_nonoption_begin;
	int		   m_nonoption_end;
public:
	OptionParser( int argc, char ** argv);

	int GetOption( const char * options);
	
	int Option( void) const;
	int Index( void) const;
	char * Argument( void) const;
};

#endif // _OPTIONPARSER_H_
