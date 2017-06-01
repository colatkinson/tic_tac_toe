IDIR = ../include
UT_IDIR = ../uthash/src
DEPS = $(IDIR)/pdf.h $(IDIR)/board.h
FLAGS = -DHPDF_SHARED
LDLIBS = -lhpdf -lpng -lz -lm -g
BDIR = ../build
OBJS = $(addprefix $(BDIR)/, main.o board.o pdf.o)

$(BDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(IDIR) -I$(UT_IDIR)

$(BDIR)/tic-tac-toe: $(OBJS)
	$(CC) -o $(BDIR)/tic-tac-toe $(OBJS) $(CFLAGS) $(FLAGS) $(LDLIBS)
