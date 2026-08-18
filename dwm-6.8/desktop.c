/* Single binary: the WM. st/dmenu run via fork from dwm; the bar is in-process. */

int dwm_main(int argc, char **argv);

int
main(int argc, char **argv)
{
	return dwm_main(argc, argv);
}
