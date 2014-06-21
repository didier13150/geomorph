noinst_LIBRARIES = libhf.a

libhf_a_SOURCES = surfadd.c img_process_dialog.c surfadd_dialog.c \
	img_process.c img_process_dialog.c subdiv1.c uniform.c subdiv1_dialog.c \
	uniform_dialog.c hf_filters.c hf_filters_dialog.c \
	draw_dialog.c draw_hf_dialog.c draw_crater_dialog.c \
	waves_dialog.c waves.c subdiv2_dialog.c subdiv2.c gl_preview.c merge_dialog.c \
	geom_transform_dialog.c erode_dialog.c craters.c view_area.c draw.c  \
	draw_tools_dialog.c camera.c hf_calc.c hf.c draw_hf.c merge.c \
	standard.map peak.map irregular.map \
	craters.h draw_crater_dialog.h draw_dialog.h draw.h draw_hf_dialog.h \
	draw_tools_dialog.h draw_hf.h \
	erode_dialog.h geom_transform_dialog.h globals.h gl_preview.h hf_filters_dialog.h \
	hf_filters.h hf.h img_process_dialog.h img_process.h merge_dialog.h subdiv1.h \
	subdiv2.h surfadd.h uniform.h view_area.h waves_dialog.h waves.h camera.h \
	fault.h fault.c fault_dialog.h fault_dialog.c swirl.h swirl.c swirl_dialog.h swirl_dialog.c erode.c erode.h wind.c wind.h wind_dialog.c wind_dialog.h \
	dialog_utilities.h dialog_utilities.c line.h line.c line_dialog.h line_dialog.c \
	fill.h fill_dialog.h fill.c fill_dialog.c render.c render.h merge.h \
	hf_wrapper.c hf_wrapper.h hf_dialog_options.c hf_dialog_options.h \
	hf_tools_dialog.c hf_tools_dialog.h hf_creation_dialog.c hf_creation_dialog.h \
	voronoi.c voronoi.h voronoi_dialog.c voronoi_dialog.h hf_calc.h

AM_CPPFLAGS =  @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@  -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 
LDADD = @GTK_LIBS@ @GTKGLEXT_LIBS@ ../utils/libutils.a ../fourier/libfourier.a
CLEANFILES = *~ *.o libhf.a
DISTCLEANFILES = .dep/*.P
