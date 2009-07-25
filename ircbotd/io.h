#ifndef IO_H_
#define IO_H_

char * get_next_line(int fd);
int get_next_char(int fd);
int write_data(int fd, char * data);
#endif /* IO_H_ */
