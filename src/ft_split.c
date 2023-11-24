/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:18:54 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/24 23:19:55 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

size_t	ft_strlen(const char *s)
{
	int	i;

	i = 0;
	if (!s)
		return (0);
	while (s[i] != '\0')
		i++;
	return ((size_t)i);
}

char	*ft_strdup(const char *s1)
{
	char	*s2;
	int		i;

	s2 = (char *)malloc((ft_strlen(s1) + 1) * sizeof(char));
	if (!s2)
		return (NULL);
	i = 0;
	while (s1[i] != '\0')
	{
		s2[i] = s1[i];
		i++;
	}
	s2[i] = '\0';
	return (s2);
}

static int	ft_count_word(char **strs, char c)
{
	int	i;
	int	trigger;
	int	j;
	int	k;

	i = 0;
	trigger = 0;
	j = 0;
	k = 0;
	while (strs[j])
	{
		while (strs[j][k])
		{
			if (strs[j][k] != c && trigger == 0)
			{
				trigger = 1;
				i++;
			}
			else if (strs[j][k] == c)
				trigger = 0;
			k++;
		}
		j++;
	}
	return (i);
}

size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	i;

	i = 0;
	if (!dst || !src)
		return (0);
	if (size > 0)
	{
		while (src[i] != '\0' && i < (size - 1))
		{
			dst[i] = src[i];
			i++;
		}
		dst[i] = '\0';
	}
	while (src[i])
		i++;
	return (i);
}

char	**ft_split(char **s, char c)
{
	char	**tab;
	int		i;
	int		j;
	int		k;
	int		start;

	tab = (char **)malloc(sizeof(char *) * (ft_count_word(s, c) + 2));
	if (!tab)
		return (NULL);
	i = 1;
	k = 0;
	while (s[i] != NULL)
	{
		j = 0;
		while (s[i][j] != '\0')
		{
			if (s[i][j] == c)
				j++;
			else
			{
				start = j;
				while (s[i][j] != c && s[i][j] != '\0')
					j++;
				tab[k] = (char *)malloc(sizeof(char) * (j - start + 1));
				if (!tab[k])
				{
					ft_free_split(tab);
					return (NULL);
				}
				ft_strlcpy(tab[k], s[i] + start, j - start + 1);
				k++;
			}
		}
		i++;
	}
	tab[k] = NULL;
	return (tab);
}
