#include "../std/stdint.h"
#include "../std/stdio.h"
#include "disk/disk.h"
#include "disk/fat.h"
#include "graphics/video.h"
#include "keyboard/keyboard.h"

disk_t global_disk;

char far *mkstr(char *str)
{
	char far *output = malloc(strlen(str) + 1);
	int i = 0;
	for (int x = 0; x < strlen(str); ++x)
	{
		output[x] = 0;
	}
	while (*str)
	{
		output[i] = *str;
		++str;
		++i;
	}
	return output;
}

void test_delay(void)
{
	// freezes teh whole OS use carefully.
	for (int x = 0; x < 10000; ++x)
	{
		for (int j = 0; j < 1200; ++j)
		{
			int xj = x + j + x + j;
			xj /= (j + x) + 1; // fun fact, division is hella slow
			xj |= xj / 2 + (j + x);
			xj &= ~xj / 2 + (j + x);
			int y = xj + x + j;
			y += 3;
			xj /= y + xj;
		}
	}
}

unsigned read_pit_count(void)
{
	unsigned count = 0;

	_asm cli;

	// al = channel in bits 6 and 7, remaining bits clear
	outb(0x43, 0x00);

	count = inb(0x40);		 // Low byte
	count |= inb(0x40) << 8; // High byte

	_asm sti;

	return count;
}

typedef struct
{
	uint32_t rows;
	uint32_t cols;
	char far *display_buffer;
	uint32_t input_index;
	char far *input_buffer;
	uint32_t cursor_x;
	uint32_t cursor_y;
	char far *current_directory;
	char far *parent_directory;
} terminal_t;

void terminal_puts(terminal_t far *term, char far *str);

terminal_t far *init_terminal()
{
	terminal_t far *term = (terminal_t far *)malloc(sizeof(terminal_t));
	term->display_buffer = malloc(TEXT_WIDTH * TEXT_HEIGHT);
	term->input_buffer = malloc(TEXT_WIDTH * TEXT_HEIGHT);
	term->current_directory = mkstr("/");
	term->parent_directory = mkstr("/");
	return term;
}

terminal_t far *delete_terminal(terminal_t far *term)
{
	free(term->display_buffer);
	free(term->input_buffer);
	free(term);
}

char old_cursor = ' ';
char new_cursor = '_';
int cursor_n_frames = 0;
void clear_terminal(terminal_t far *term)
{
	int x = 0;
	int y = 0;
	for (int i = 0; i < TEXT_WIDTH * TEXT_HEIGHT; ++i)
	{
		term->display_buffer[i] = 255;
		if (term->display_buffer[i] == 245)
		{
			continue;
		}
		draw_character(0, ' ', x, y);
		++x;
		if (x > TEXT_WIDTH)
		{
			y++;
			x = 0;
		}
	}
}

void draw_cursor(terminal_t far *term)
{
	if (cursor_n_frames > 3500)
	{
		char temp = old_cursor;
		term->display_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			new_cursor;
		old_cursor = new_cursor;
		new_cursor = temp;
		cursor_n_frames = 0;
		return;
	}
	++cursor_n_frames;
}

int far_strlen(char far *str)
{
	int count = 0;
	while (*str)
	{
		++str;
		++count;
	}
	return count;
}

bool far_strcmp(char far *str, char far *str2)
{
	int l = strlen(str);
	int l2 = strlen(str2);
	// I could swap the variables but I'm really lazy
	if (l2 > l)
	{
		while (*str)
		{
			if (*str != *str2)
			{
				return false;
			}
			++str;
			++str2;
		}
	}
	else
	{
		while (*str2)
		{
			if (*str != *str2)
			{
				return false;
			}
			++str;
			++str2;
		}
	}
	return true;
}

char far **split(char delim, char far *str, int *count)
{
	int num = 1;
	int i = 0;
	while (str[i])
	{
		if (str[i] == delim)
			++num;
		++i;
	}
	char far **buffer = malloc(sizeof(char far **) * num);
	i = 0;
	int idx = 0;
	char far *temp = malloc(200);
	int tempidx = 0;
	while (str[i])
	{
		if (str[i] == 255)
		{
			++i;
			continue;
		}
		if (str[i] == delim)
		{
			buffer[idx] = temp;
			temp = malloc(200); // yes a new one does have to be allocated
			memset(temp, 0, 200);
			tempidx = 0;
			++idx;
			++i;
			continue;
		}
		temp[tempidx++] = str[i];
		++i;
	}
	buffer[idx] = temp;
	return buffer;
}

void far_puts(char far *str)
{
	while (*str)
	{
		_putc(*str, 15);
		++str;
	}
}

void to_upper(char far *str)
{
	int len = far_strlen(str);
	for (int i = 0; i < len; ++i)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
		{
			str[i] = 'A' + (str[i] - 'a');
		}
	}
}

void wait_for_key(char key)
{
	while (true)
	{
		char k;
		if (getk(&k))
		{
			if (k == key)
				return;
		}
	}
}

void handle_command(terminal_t far *term)
{
	char far *buffer = malloc(TEXT_WIDTH + 1);
	memset(buffer, 0, TEXT_WIDTH + 1);
	buffer[TEXT_WIDTH] = 0;
	for (int i = 0; i < TEXT_WIDTH; ++i)
	{
		if (term->input_buffer[i] == 255 || term->input_buffer[i] == 245)
			continue;
		buffer[i] = term->input_buffer[i];
	}
	if (far_strcmp(buffer, mkstr("cat")))
	{
		int argc = 0;
		char far **buf = split(' ', buffer, &argc);
		if (argc == 1)
		{
			printf("Please provide atleast 1 argument. \n");
			wait_for_key('q');
		}
		char far *path_buf = malloc(far_strlen(term->current_directory) + far_strlen(buf[1]) + 2);
		memset(path_buf, 0, far_strlen(term->current_directory) + far_strlen(buf[1]) + 2);
		memcpy(path_buf, term->current_directory, far_strlen(term->current_directory));
		if (!far_strcmp(term->current_directory, "/"))
		{
			memcpy((char far *)(path_buf + far_strlen(term->current_directory)), mkstr("/"), 1);
			memcpy((char far *)(path_buf + far_strlen(term->current_directory) + 1), buf[1], far_strlen(buf[1]));
		}
		else
		{
			memcpy((char far *)(path_buf + far_strlen(term->current_directory)), buf[1], far_strlen(buf[1]));
		}
		char buffer[1024];
		char str[100];
		memset(str, 0, 100);
		for (int i = 0; i < far_strlen(path_buf); ++i)
		{
			str[i] = path_buf[i];
		}
		FILE far *fp = fopen(&global_disk, str);
		fread(&global_disk, fp, 1023, buffer);
		printf("\n%s\n", buffer);
		fclose(fp);
		wait_for_key('q');
	}
	else if (far_strcmp(buffer, mkstr("cd")))
	{
		int argc = 0;
		char far **buf = split(' ', buffer, &argc);
		if (argc == 1)
		{
			free(term->current_directory);
			free(term->parent_directory);
			term->parent_directory = mkstr("/");
			term->current_directory = mkstr("/");
			wait_for_key('q');
			free(buf[0]);
			free(buf);
			return;
		}
		char str[100];
		memset(str, 0, 100);
		for (int i = 0; i < far_strlen(buf[1]); ++i)
		{
			str[i] = buf[1][i];
		}
		if (strcmp(str, "."))
		{
			for (int i = 0; i < argc; ++i)
			{
				free(buf[i]);
			}
			free(buf);
			return;
		}
		if (strcmp(str, "../") || strcmp(str, ".."))
		{
			for (int i = 0; i < 11; ++i)
			{
				term->current_directory[i] = term->parent_directory[i];
			}

			for (int i = 0; i < argc; ++i)
			{
				free(buf[i]);
			}
			free(buf);
			return;
		}
		term->parent_directory = term->current_directory;
		term->current_directory = malloc(far_strlen(buf[1]) + 1);
		memset(term->current_directory, 0, far_strlen(buf[1]) + 1);
		memcpy(term->current_directory, buf[1], far_strlen(buf[1]));
		FILE far *fp = fopen(&global_disk, str);

		if (!fp->IsDirectory)
		{
			printf("\"");
			far_puts(buf[1]);
			printf("\" is not a directory.\n");
			for (int i = 0; i < argc; ++i)
			{
				free(buf[i]);
			}
			free(buf);
			fclose(fp);
			wait_for_key('q');

			return;
		}

		fclose(fp);
		for (int i = 0; i < argc; ++i)
		{
			free(buf[i]);
		}
		free(buf);
	}
	else if (far_strcmp(buffer, mkstr("ls")))
	{
		char str[12];
		memset(str, 0, 12);
		int i = 0;
		while (term->current_directory[i])
		{
			str[i] = term->current_directory[i];
			++i;
		}
		FILE far *root = fopen(&global_disk, str);
		fdir_entry dir;
		int lim = 4;
		x86_Set_Cursor_Pos(0, term->cursor_y + 1);
		while (fread_entry(&global_disk, root, &dir) && lim--)
		{
			char name[12];
			name[11] = 0;
			memcpy(name, dir.Name, 11);
			printf("%s\n", name);
		}
		fclose(root);
		wait_for_key('q');
	}
}

void display_terminal(terminal_t far *term)
{
	draw_cursor(term);
	for (int y = 0; y < TEXT_HEIGHT; ++y)
	{
		for (int x = 0; x < TEXT_WIDTH; ++x)
		{
			if (term->display_buffer[x + (y * TEXT_WIDTH)] == '\t')
			{
				draw_character(15, ' ', x, y);
				continue;
			}
			if (term->display_buffer[x + (y * TEXT_WIDTH)] == 255)
			{
				draw_character(15, ' ', x, y);
			}
			if (term->display_buffer[x + (y * TEXT_WIDTH)] == 0 || term->display_buffer[x + (y * TEXT_WIDTH)] == 245)
			{
				continue;
			}
			draw_character(15, term->display_buffer[x + (y * TEXT_WIDTH)], x,
						   y);
		}
	}
}

void terminal_puts(terminal_t far *term, char far *str)
{
	int x = term->cursor_x;
	for (; x - term->cursor_x < far_strlen(str); ++x)
	{
		term->display_buffer[x + (term->cursor_y * TEXT_WIDTH)] = str[x];
	}
	display_terminal(term);
	for (x = term->cursor_x; x - term->cursor_x < far_strlen(str); ++x)
	{
		term->display_buffer[x + (term->cursor_y * TEXT_WIDTH)] = 245;
	}
}

uint32_t input_last_line_length(terminal_t far *term)
{
	uint32_t x = TEXT_WIDTH;
	uint32_t y = term->cursor_y;
	while (term->input_buffer[x + (y * TEXT_WIDTH)] == 255 ||
		   term->input_buffer[x + (y * TEXT_WIDTH)] == 0)
	{
		--x;
	}
	return x + 1;
}

void handle_key_press(terminal_t far *term, char key)
{
	if (term->cursor_x == TEXT_WIDTH - 1 && key != '\n')
	{
		return;
	}
	if (key == ' ')
	{
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key >= 'a' && key <= 'z')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = (key - 'a') + 'A';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key >= '0' && key <= '9')
	{
		char *upper = "!@#$%^&*()";
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			if (key == '0')
			{
				key = ')';
			}
			else
			{
				key = upper[(key - '0') - 1];
			}
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '/')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '?';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '=')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '+';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '-')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '_';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '[')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '{';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == ']')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '}';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == ',')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '<';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '.')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '>';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '\t')
	{
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			'\t';
		term->cursor_x++;
	}
	else if (key == ';')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = ':';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == '\'')
	{
		if (keyboard_state[LEFT_SHIFT] || keyboard_state[RIGHT_SHIFT])
		{
			key = '"';
		}
		term->input_buffer[term->cursor_x + (term->cursor_y * TEXT_WIDTH)] =
			key;
		term->cursor_x++;
	}
	else if (key == BACKSPACE)
	{
		if (term->cursor_y == 0 && term->cursor_x <= 0)
		{
			memcpy(term->display_buffer, term->input_buffer,
				   TEXT_WIDTH * TEXT_HEIGHT);
			return;
		}
		else if (term->cursor_x <= 0)
		{
			term->input_buffer[(term->cursor_x +
								(term->cursor_y * TEXT_WIDTH))] = 255;
			term->cursor_y--;
			term->cursor_x = input_last_line_length(term);
			return;
		}
		term->cursor_x--;
		term->input_buffer[(term->cursor_x + (term->cursor_y * TEXT_WIDTH))] =
			255;
	}
	else if (key == '\n')
	{
		handle_command(term);
		clear_terminal(term);
		memset(term->input_buffer, 255, TEXT_WIDTH);
		term->cursor_x = 0;
		term->cursor_y = 0;
	}
	else
	{
	}
	memcpy(term->display_buffer, term->input_buffer, TEXT_WIDTH * TEXT_HEIGHT);
}

void update_terminal(terminal_t far *term)
{
	char key;
	if (getk(&key))
	{
		handle_key_press(term, key);
		char old_key = key;
		while (old_key == key && getk(&key))
			;
	}
}

int _main(uint16_t bootDrive)
{
	init_graphics();
	init_allocator();
	init_keyboard();
	init_disk(&global_disk, bootDrive);
	init_fat(&global_disk);
	x86_Set_Cursor_Pos(0, 0);
	terminal_t far *term = init_terminal();
	while (true)
	{
		if (keyboard_state[ESCAPE])
		{
			x86_Reboot();
		}
		display_terminal(term);
		update_keyboard();
		update_terminal(term);
	}
	return 0;
}

void _cdecl cstart_(uint16_t bootDrive)
{
	_main(bootDrive);
	x86_Reboot();
}
