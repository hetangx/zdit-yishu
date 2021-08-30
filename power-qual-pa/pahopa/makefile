###########################################################
################### Makefile Template #####################
# Modify the definitions belows according to              #
# your project requirements                               #
###########################################################

# object, should be edited for each programme
BIN = pahopa
LIB = 

# compile tool
CC = gcc
#CC = gcc
AR = ar
#AR = ar

# compile flags
# CFLAGS = -Werror
CFLAGS = 

# link flags
# if gcc version < 4.6, using below
# LFLAGS = -m32 -m elf_i386
# if gcc version > 4.6, using below
LFLAGS = -lpaho-mqtt3c -lpaho-mqtt3a -lm -lsqlite3

# include flags
INCFLAGS = 

###########################################################
########## Do not modify belows if not necessary ##########
###########################################################

# make rules

# directory definitions
PRJ_DIR = $(shell pwd)
BIN_DIR = $(PRJ_DIR)/bin
LIB_DIR = $(PRJ_DIR)/lib
BLD_DIR = $(PRJ_DIR)/build
INC_DIR = $(PRJ_DIR)/include
SRC_DIR_BIN = $(PRJ_DIR)/src
SRC_DIR_LIB = $(PRJ_DIR)/src

# tools
SED	= sed
RM  = rm -rf
CP  = cp
MKDIR = mkdir -p
ECHO = echo

# default flags
INCFLAGS += -I. -I$(SRC_DIR_BIN) -I$(SRC_DIR_LIB) -I$(INC_DIR) 
CFLAGS   += -O2 -fno-strict-aliasing -Wall -DLINUX $(INCFLAGS) 
LFLAGS   += -lpthread -lrt 

# targets
ifeq ($(BIN),)
BBIN = 
else
BBIN = $(BIN_DIR)/$(BIN)
endif
ifeq ($(LIB),)
BLIB = 
else
BLIB = $(LIB_DIR)/$(LIB)
endif

# directories
SRC_DIRS_BIN = $(shell find $(SRC_DIR_BIN) -type d)
SRC_DIRS_LIB = $(shell find $(SRC_DIR_LIB) -type d)
BLD_DIRS = $(shell echo $(SRC_DIRS_BIN) | sed "s $(PRJ_DIR) $(BLD_DIR) g")
BLD_DIRS += $(shell echo $(SRC_DIRS_LIB) | sed "s $(PRJ_DIR) $(BLD_DIR) g")

# souce files and object files
BIN_SRCS = $(shell find $(SRC_DIR_BIN) -name "*.c")
LIB_SRCS = $(shell find $(SRC_DIR_LIB) -name "*.c")
BOOBJS   = $(addsuffix .o, $(basename ${BIN_SRCS}))
LOOBJS   = $(addsuffix .o, $(basename ${LIB_SRCS}))
BOBJS    = $(shell echo $(BOOBJS) | sed "s $(PRJ_DIR) $(BLD_DIR) g")
LOBJS    = $(shell echo $(LOOBJS) | sed "s $(PRJ_DIR) $(BLD_DIR) g")
BDEPS    = $(addsuffix .d, $(basename $(BOBJS)))
LDEPS    = $(addsuffix .d, $(basename $(LOBJS)))

CPBIN = $(GLBBINDIR)/$(BIN)
CPLIB = $(GLBLIBDIR)/$(LIB)

TREE = $(BLD_DIR)/.tree

.PHONY:	clean depend print_var

DIRHOLDERS = $(foreach DIRTEMP, $(BLD_DIRS), $(DIRTEMP)/.holder)
DIRHOLDERS += $(BIN_DIR)/.holder $(LIB_DIR)/.holder

# targets
#TARGET: $(BLIB) $(BBIN) $(CPLIB) $(CPBIN)
TARGET: $(BLIB) $(BBIN)
	@$(ECHO) Complete building $(BIN) $(LIB)

$(BBIN): $(BOBJS)
	$(CC) -o $@ $^ $(LFLAGS)

$(BLIB): $(LOBJS)
	$(AR) r $@ $^

$(CPBIN): $(BBIN)
	$(CP) $^ $@

$(CPLIB): $(BLIB)
	$(CP) $^ $@

$(BLD_DIR)/%.o: %.c $(BLD_DIR)/%.d
	@$(ECHO) compiling $<
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/%.d: %.c $(TREE)
	@$(ECHO) generating dependency for $<
	$(CC) -M $(CFLAGS) $< > $(BLD_DIR)/$*.dd
	$(SED) -e '1s/\(^\)/$(subst /,\/,$(dir $@))/' $(BLD_DIR)/$*.dd > $(BLD_DIR)/$*.d

$(TREE): $(DIRHOLDERS)
	@$(ECHO) done > $@

$(DIRHOLDERS): %/.holder :
	@$(ECHO) Making build dir $*
	@$(MKDIR) $*
	@$(ECHO) placeholder for $* > $@

depend: $(LDEPS) $(BDEPS)

clean:
	@$(RM) $(BIN_DIR)/*
	@$(RM) $(LIB_DIR)/*
	@$(RM) $(BLD_DIR)/*

print_var:
	@$(ECHO) lib srcs: $(LIB_SRCS)
	@$(ECHO) bin srcs: $(BIN_SRCS)
	@$(ECHO) lib objs: $(LOBJS)
	@$(ECHO) bin objs: $(BOBJS)

-include $(BDEPS)
-include $(LDEPS)

