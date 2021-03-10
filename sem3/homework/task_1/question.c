int sameFile (int fd_1, int fd_2)
{
	struct stat stat_1 = {}, stat_2 = {};

	if (fstat (fd_1, &stat_1) < 0)
	{
		return -1;
	}

	if (fstat (fd_2, &stat_2) < 0)
	{
		return -1;
	}

	if (stat_1.st_dev == stat_2.st_dev && stat_1.st_ino == stat_2.st_ino)
		return 0;
	else 
		return 1;
}
