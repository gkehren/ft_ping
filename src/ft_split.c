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

	s2 = (char *)malloc(ft_strlen(s1) + 1 * sizeof(char));
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

static void	ft_split_word(char *dest, char const *src, char c)
{
	int	i;

	i = 0;
	while (src[i] != c && src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

static int	ft_count_word(char const *str, char c)
{
	int	i;
	int	trigger;

	i = 0;
	trigger = 0;
	while (*str)
	{
		if (*str != c && trigger == 0)
		{
			trigger = 1;
			i++;
		}
		else if (*str == c)
			trigger = 0;
		str++;
	}
	return (i);
}

char	**ft_split(char const *s, char c)
{
	char	**tab;
	int		i;
	int		j;
	int		k;

	tab = (char **)malloc(sizeof(char *) * (ft_count_word(s, c) + 1));
	i = 0;
	k = 0;
	while (s[i] != '\0')
	{
		if (s[i] == c)
			i++;
		else
		{
			j = 0;
			while (s[i + j] != c && s[i + j] != '\0')
				j++;
			tab[k] = (char *)malloc(j + 1 * sizeof(char));
			ft_split_word(tab[k], s + i, c);
			i += j;
			k++;
		}
	}
	tab[k] = NULL;
	return (tab);
}
