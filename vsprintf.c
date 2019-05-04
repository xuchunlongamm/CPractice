/*************************************************************************
	> File Name: vsprintf.c
	> Author:爱瑞的LS 
	> Mail:2939720340@qq.com 
	> Created Time: 2019年05月04日 星期六 11时46分51秒
 ************************************************************************/

#include <stdio.h>
  /*
      *  linux/kernel/vsprintf.c
      *
      *  Copyright (C) 1991, 1992  Linus Torvalds
      */
      /* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
      /*
      * Wirzenius wrote this portably, Torvalds fucked it up :-)
      */
      #include <stdarg.h>
      #include <linux/types.h>
      #include <linux/string.h>
      #include <linux/ctype.h>
      //简单的字符串分隔 字符串到无符号长整型转化
      unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
      {
      unsigned long result = 0,value;
      //如果未指明进制
      if (!base) {
      //初始化为10进制
      base = 10;
      //如果cp以0开始则为8进制
      if (*cp == '0') {
      //8进制
      base = 8;
      //下一字符
      cp++;
      //如果是x并且检测第二个字符是否为十六进制数字
      if ((*cp == 'x') && isxdigit(cp[1])) {
      //则为16进制
      cp++;
      base = 16;
      }
      }
      }
      //明确进制，是否为十六进制数字，如果是，那么是数字，还是是字符，处理不同情况后
      while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
      ? toupper(*cp) : *cp)-'A'+10) < base) {
      result = result*base + value;//进行计算
      cp++;
      }
      //判断是否结束转化
      if (endp)
      *endp = (char *)cp;
      return result;//返回结果
      }
      /* we use this so that we can do without the ctype library */
      //判断字符c是否为数字
      #define is_digit(c)    ((c) >= '0' && (c) <= '9')
      //字符串转为整型，
      static int skip_atoi(const char **s)
      {
      int i=0;
      while (is_digit(**s))
      i = i*10 + *((*s)++) - '0';
      return i;
      }
      #define ZEROPAD    1        /* pad with zero */                           //用0填补
      #define SIGN    2        /* unsigned/signed long */                    //无符号/符号长整型
      #define PLUS    4        /* show plus */                               //显示*号
      #define SPACE    8        /* space if plus */                           //如果+号，则空格
      #define LEFT    16        /* left justified */                          //左侧调整
      #define SPECIAL    32        /* 0x */                                      //0x
      #define SMALL    64        /* use 'abcdef' instead of 'ABCDEF' */        //小写替代大写
      //除操作
      #define do_div(n,base) ({ \
      int __res; \
      __asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
      __res; })
      //将字符转为指定进制的字符串
      static char * number(char * str, int num, int base, int size, int precision
      ,int type)
      {
      char c,sign,tmp[36];
      const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      int i;
      //根据类型type决定格式
      if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";//是否使用小写
      if (type&LEFT) type &= ~ZEROPAD;                              //左侧是否填零
      if (base<2 || base>36)                                        //进制判断
      return 0;
      c = (type & ZEROPAD) ? '0' : ' ' ;                            //如果用0填补，则补零，否则用空格
      if (type&SIGN && num<0) {                                     //格式要求有符号输出，并且小于0
      sign='-';                                                 //负号
      num = -num;
      } else
      sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);        //正数
      if (sign) size--;                                             //如果带符号，则宽度减1
      if (type&SPECIAL)                                             //如果是特殊转换，16进制宽度减2,8进制减1
      if (base==16) size -= 2;
      else if (base==8) size--;
      i=0;                                                          //
      if (num==0)                                                   //如果num是0
      tmp[i++]='0';                                             //结果
      else while (num!=0)                                           //如果不是0
      tmp[i++]=digits[do_div(num,base)];                        //根据进制转化为字符形式
      if (i>precision) precision=i;                                 //确定精度
      size -= precision;
      if (!(type&(ZEROPAD+LEFT)))                                   //不是补零和左对齐
      while(size-->0)
      *str++ = ' ';
      if (sign)
      *str++ = sign;
      if (type&SPECIAL)                                             //处理不同进制的输出前缀
      if (base==8)
      *str++ = '0';
      else if (base==16) {
      *str++ = '0';
      *str++ = digits[33];
      }
      if (!(type&LEFT))                                            //如果不是左对齐
      while(size-->0)
      *str++ = c;
      while(i<precision--)                                         //i为数值num的长度，如果此长度小于指定精度，则补零
      *str++ = '0';
      while(i-->0)                                                 //将转换好的数据填入str中
      *str++ = tmp[i];
      while(size-->0)                                             //左对齐，空格补齐
      *str++ = ' ';
      return str;
      }
      //格式化字符串
      int vsprintf(char *buf, const char *fmt, va_list args)
      {
      int len;
      int i;
      char * str;
      char *s;
      int *ip;
      int flags;        /* flags to number() */
      int field_width;    /* width of output field */
      int precision;        /* min. # of digits for integers; max
      number of chars for from string */
      int qualifier;        /* 'h', 'l', or 'L' for integer fields */
      //首先将字符指针指向buf，然后扫描格式字符串，对各个格式转换进行相应的处理
      for (str=buf ; *fmt ; ++fmt) {
      //从fmt中扫描%，寻找格式转换字符串的开始，如果不是格式指示的字符，则直接依次放入str中
      if (*fmt != '%') {
      *str++ = *fmt;
      continue;
      }
      /* process flags */
      //获取格式指示的标志类型
      flags = 0;
      repeat:
      ++fmt;        /* this also skips first '%' */
      switch (*fmt) {
      case '-': flags |= LEFT; goto repeat;
      case '+': flags |= PLUS; goto repeat;
      case ' ': flags |= SPACE; goto repeat;
      case '#': flags |= SPECIAL; goto repeat;
      case '0': flags |= ZEROPAD; goto repeat;
      }
      //确定转换宽度
      /* get field width */
      field_width = -1;
      if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
      else if (*fmt == '*') {
      /* it's the next argument */
      //这里有bug，需要加入一行代码 ++fmt;
      field_width = va_arg(args, int);
      if (field_width < 0) {
      field_width = -field_width;
      flags |= LEFT;
      }
      }
      //确定转换精读
      /* get the precision */
      precision = -1;
      if (*fmt == '.') {
      ++fmt;    
      if (is_digit(*fmt))
      precision = skip_atoi(&fmt);
      else if (*fmt == '*') {
      /* it's the next argument */
      precision = va_arg(args, int);
      }
      if (precision < 0)
      precision = 0;
      }
      //获取长度修饰
      /* get the conversion qualifier */
      qualifier = -1;
      if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
      qualifier = *fmt;
      ++fmt;
      }
      //开始分析转换指示符
      switch (*fmt) {
      case 'c'://表示对应的参数是字符
      if (!(flags & LEFT)) //是否左对齐
      while (--field_width > 0)
      *str++ = ' ';
      *str++ = (unsigned char) va_arg(args, int);
      while (--field_width > 0)
      *str++ = ' ';
      break;
      //对应参数是字符串
      case 's':
      s = va_arg(args, char *);
      if (!s)
      s = "<NULL>";
      len = strlen(s);
      if (precision < 0)
      precision = len;
      else if (len > precision)
      len = precision;
      if (!(flags & LEFT))
      while (len < field_width--)
      *str++ = ' ';
      for (i = 0; i < len; ++i)
      *str++ = *s++;
      while (len < field_width--)
      *str++ = ' ';
      break;
      //对应参数是8进制字符
      case 'o':
      str = number(str, va_arg(args, unsigned long), 8,
      field_width, precision, flags);
      break;
      //对应参数是指针类型
      case 'p':
      if (field_width == -1) {
      field_width = 8;
      flags |= ZEROPAD;
      }
      str = number(str,
      (unsigned long) va_arg(args, void *), 16,
      field_width, precision, flags);
      break;
      //对应参数是16进制，x是小写格式，X是大写格式
      case 'x':
      flags |= SMALL;
      case 'X':
      str = number(str, va_arg(args, unsigned long), 16,
      field_width, precision, flags);
      break;
      //参数是整数的处理
      case 'd':
      case 'i':
      flags |= SIGN;
      case 'u':
      str = number(str, va_arg(args, unsigned long), 10,
      field_width, precision, flags);
      break;
      //如果指示符是n，即\n，则表示要把目前为止转换输出字符保存到对应参数指针指定的位置中
      case 'n':
      ip = va_arg(args, int *);
      *ip = (str - buf);
      break;
      //如果不是以%，则转换格式出错，处理后退出
      default:
      if (*fmt != '%')
      *str++ = '%';
      if (*fmt)
      *str++ = *fmt;
      else
      --fmt;
      break;
      }
      }
      //字符串结束符
      *str = '\0';
      return str-buf;//长度
      }
      //格式化输出
      int sprintf(char * buf, const char *fmt, ...)
      {
      //变长参数
      va_list args;
      int i;
      va_start(args, fmt);
      i=vsprintf(buf,fmt,args);
      va_end(args);
      return i;
      }
