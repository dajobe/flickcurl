#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include <flickcurl.h>


#undef CONFIG_DEBUG

int
read_ini_config(const char* filename, const char* application,
                void* user_data, set_config_var_handler handler)
{
  FILE* fh;
  char buf[256];
  int in_section=0;
  
  if(access((const char*)filename, R_OK))
    return 1;
  
  fh=fopen(filename, "r");
  if(!fh)
    return 1;

  while(!feof(fh)) {
    size_t len;
    char *line;
    char *p;
    
    for(line=buf, len=0; !feof(fh); ) {
      int c=fgetc(fh);
      if(c == '\n')
        break;
      *line++=c;
      len++;
    }
    *line='\0';

    if(!len)
      continue;

#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 1 >>%s<<\n", line);
#endif
    
    /* remove leading spaces */
    for(line=buf; 
        *line && (*line==' ' || *line == '\t');
        line++, len--)
      ;

#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 2 >>%s<<\n", line);
#endif

    /* skip if empty line or all white space OR starts with a comment */
    if(!*line || *line == '#')
      continue;
    
    if(line[len-1]=='\n')
      line[(len--)-1]='\0';
    
#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 3 >>%s<<\n", line);
#endif

    /* Wait for a line '['application']' */
    if(!in_section) {
      if(*line == '[' &&
         !strncmp(line+1, application, strlen(application)) &&
         line[len-1] == ']')
        in_section=1;
      continue;
    }

    /* End at a line starting with '[' */
    if(*line == '[')
      break;

#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 4 >>%s<<\n", line);
#endif

    p=strchr(line, '=');
    if(p) {
      *p='\0';
#ifdef CONFIG_DEBUG    
      fprintf(stderr, "Found key '%s' value '%s'\n", line, p+1);
#endif
      if(handler)
        handler(user_data, line, p+1);
    }
  }
  fclose(fh);

  return 0;
}
