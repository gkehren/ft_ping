/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:18:54 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 16:27:25 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

static int	ft_count_word(char **strs, char c)
{
	int		i;
	int		j;
	int		count;

	count = 0;
	i = 0;
	while (strs[i] != NULL)
	{
		j = 0;
		while (strs[i][j] != '\0')
		{
			if (strs[i][j] == c)
			{
				count++;
				while (strs[i][j] != '\0' && strs[i][j] != c)
					j++;
			}
			j++;
		}
		i++;
		count++;
	}
	return (count);
}

char	*ft_extract_substring(char *s, char c, int *start, int *end)
{
	while (s[*start] && s[*start] == c)
		(*start)++;
	*end = *start;
	while (s[*end] && s[*end] != c)
		(*end)++;
	return (ft_strndup(s + *start, *end - *start));
}

char	**ft_split(char **s, char c)
{
	char	**tab;
	int		i;
	int		j;
	int		k;
	int		end;

	tab = (char **)malloc(sizeof(char *) * ft_count_word(s, c));
	i = 1;
	k = 0;
	while (s[i])
	{
		j = 0;
		while (s[i][j])
		{
			tab[k] = ft_extract_substring(s[i], c, &j, &end);
			if (!tab[k++])
				return (ft_free_split(tab), NULL);
			j = end;
		}
		i++;
	}
	tab[k] = NULL;
	return (tab);
}
