
# Colors
_RED	=	\e[31m
_GREEN	=	\e[32m
_YELLOW	=	\e[33m
_NC		=	\e[0m

NAME    = ft_ping
RM      = rm      -rf
CC      = clang
FLAGS   = -Wall -Wextra -Werror -g
DIR_INC = -I ./includes/

SRCS 	:= 	main.c \
			dns_lookup.c \
			init.c \
			print.c \
			opt.c \
			verbose.c \
			utils.c

DIR_SRCS 	= ./srcs

DIR_OBJ 	= obj

OBJS        := $(addprefix ${DIR_OBJ}/, ${SRCS:.c=.o})

DEPS        =  $(OBJS:.o=.d)

all: $(NAME)

-include $(DEPS)

$(DIR_OBJ)/%.o:	$(DIR_SRCS)/%.c
	@mkdir -p $(dir $@)
	@printf "$(_YELLOW)Compiling $< $(_NC)\n"
	@$(CC) $(FLAGS) $(DIR_INC) -o $@ -c $< -MMD

$(NAME): $(OBJS)
	@printf "$(_GREEN)Generating $(NAME) $(_NC)\n"
	@$(CC) $(FLAGS) $(DIR_INC) $(OBJS) -o $(NAME) -lm

clean:
	@$(RM) $(DIR_OBJ)
	@printf "$(_RED)Deleting objects files $(NAME) $(_NC)\n"

fclean:		clean
	@printf "$(_RED)Deleting $(NAME) $(_NC)\n"
	@$(RM) $(NAME)

re:	fclean all