TOP=.
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

PROD_HOST += caCamcom
LEX=flex -i
YACC=bison -y -d -v

caCamcom_SRCS += camcom_yacc.c
caCamcom_SRCS += camcom_lex.c
caCamcom_SRCS  += camcom_support.c
caCamcom_LIBS	+= $(EPICS_BASE_HOST_LIBS)


include $(TOP)/configure/RULES
include $(TOP)/configure/RULES_TOP
#----------------------------------------
#  ADD RULES AFTER THIS LINE

