/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   micro.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmahe <lmahe@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/02 09:48:00 by lmahe             #+#    #+#             */
/*   Updated: 2024/02/04 11:29:19 by lmahe            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

int	error(char *str, char *name)
{
	write(2, str, strlen(str));
	if (name)
		write(2, name, strlen(name));
	write(2, "\n", 1);
	return (1);
}

void	fork_error(int *fd)
{
	if (fd)
	{
		close(fd[0]);
		close(fd[1]);
	}
	error("error: fatal", NULL);
	exit (1);
}

int	next_pipe(char **argv, int argc)
{
	int	j = 0;

	while (j < argc)
	{
		if (!strcmp(argv[j], "|"))
			return (j);
		j++;
	}
	return (0);
}

int	next_newl(char **argv, int argc)
{
	int	j = 0;

	while (j < argc)
	{
		if (!strcmp(argv[j], ";"))
			return (j);
		j++;
	}
	return (argc);
}


int	ft_cd(char **argv, int argc)
{
	if (argc != 2)
		return (error("error: cd: bad arguments", NULL));
	if (chdir(argv[1]))
		return (error("error: cd: cannot change directory to ", argv[1]));
	return (0);
}

int	exec_cmd(char **argv, char **env, int argc, int *oldfd, int *fd)
{
	int	id;

	if (!strcmp(argv[0], "cd"))
		{
			if (ft_cd(argv, argc))
				return (1);
			return (0);
		}
	if ((id = fork()) < 0)
		fork_error(fd);
	if (id == 0)
	{
		argv[argc] = 0;
		if (oldfd)
		{
			close(oldfd[1]);
			if ((dup2(oldfd[0], STDIN_FILENO)) < 0)
			{
				close(oldfd[0]);
				fork_error(fd);
			}
			close(oldfd[0]);
		}
		if (fd)
		{
			close(fd[0]);
			if ((dup2(fd[1], STDOUT_FILENO)) < 0)
			{
				close(fd[1]);
				fork_error(oldfd);
			}
		}
		execve(argv[0], argv, env);
		error("error: cannot execute ", argv[0]);
		exit (EXIT_FAILURE);
	}
	return (0);
}

int	exec_line(char **argv, char **env, int i, int *oldfd)
{
	int	j = 0;
	int	fd[2];

	if (i < 0)
		return (0);
	if ((j = next_pipe(argv, i)) && pipe(fd))
		return (error("error: fatal", NULL));
	if (j)
		exec_cmd(argv, env, j, oldfd, fd);
	else if (j == 0)
	{
		exec_cmd(argv, env, i, oldfd, NULL);
		j = i;
	}
	if (oldfd)
	{
		close(oldfd[0]);
		close(oldfd[1]);
		oldfd = NULL;
	}
	if (j < i)
	{
		return (exec_line(argv + j + 1, env, i - j - 1, fd));
	}
	else
		return (0);
}

int	main(int argc, char **argv, char **env)
{
	int	i = 0;
	int	old_argc = argc - 1;
	argc --;
	argv = argv + 1;
	if (argc < 1)
		return (0);
	while (i < old_argc)
	{
		i = next_newl(argv, argc);
		exec_line(argv, env, i, NULL);
		while (wait(NULL) != -1)
			continue;
		if (i == argc || i == argc - 1)
			break ;
		argv += i + 1;
		argc -= i + 1;
	}
	return (0);
}
