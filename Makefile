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
			utils.c

DIR_SRCS 	= ./srcs

DIR_OBJ 	= obj

OBJS        := $(addprefix ${DIR_OBJ}/, ${SRCS:.c=.o})

DEPS        =  $(OBJS:.o=.d)

all: $(NAME)

-include $(DEPS)

$(DIR_OBJ)/%.o:	$(DIR_SRCS)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) $(DIR_INC) -o $@ -c $< -MMD

$(NAME): $(OBJS)
	@$(CC) $(FLAGS) $(DIR_INC) $(OBJS) -o $(NAME) -lm
	@printf "$(_GREEN)Generating $(NAME) $(_NC)\n"

clean:
	@$(RM) $(DIR_OBJ)
	@printf "$(_GREEN)Deletes objects files $(NAME) $(_NC)\n"

fclean:		clean
	@$(RM) $(NAME)
	@printf "$(_GREEN)Delete $(NAME) $(_NC)\n"

re:	fclean all