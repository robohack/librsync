# -*- makefile-bsdmake -*-

#	Common/shared setup for BSD Makefiles
#
# N.B.:  Nothing in here should be changed just to build this project.
#
# (Some settings can be overridden on the command-line or from environment
# variables though...)

# XXX see SJG's SSLrsh release for simple portability hacks via Make
# e.g. in particular the use of .-include "$Makefile.${MACHINE}.inc" etc.

PACKAGE =	rsync		# n.b.:  NOT 'librsync' -- just the lib name

LIBRSYNC_MAJOR =	2
LIBRSYNC_MINOR =	0
LIBRSYNC_MICRO =	2

LIBRSYNC_VERSION =	${LIBRSYNC_MAJOR}.${LIBRSYNC_MINOR}.${LIBRSYNC_MICRO}

CPPFLAGS +=		-DVERSION='"${LIBRSYNC_VERSION}"'

# where to find libpopt and its headers...
#
# XXX this could be smarter, but it works for pkgsrc and ports in the "normal"
# places, plus Homebrew on OSX.
#
.if exists(/usr/pkg)
POPT_INCLUDE =	 /usr/pkg/include
POPT_LIB =	 /usr/pkg/lib
.elif exists(/opt/pkg)
POPT_INCLUDE =	 /opt/pkg/include
POPT_LIB =	 /opt/pkg/lib
.else
POPT_INCLUDE =	 /usr/local/include
POPT_LIB =	 /usr/local/lib
.endif
#INTL_INCLUDE =	 /usr/pkg/include
#INTL_LIB =	 /usr/pkg/lib

# For testing and debugging -- at least on sane systems, or with 'bmake'
#
# (note these can also be overridden on the command line)
#
# (note:  bmake puts optimizer flags in DBG)
#
#DBG ?= -g
#OPTIM ?= -O0

NOGCCERROR = 1
WARNS = 0

# N.B.:  testing in the build or 'install' (distribution) directories, or making
# a distribution that can be un-packed on a target system in any arbitrary
# hierarchy (/usr/local, /opt, /usr/pkg, etc.)  is "hard" (i.e. annoyingly more
# difficult than it should be) without dancing with the dynamic linker.  (Or
# suffering the horrors of even worse monstrosities, such as "GNU libtool"!)
#
# Setting LDSTATIC to the linker flag which forces static linking is the easy
# and best fix on most platforms which support full static linking.
#
#LDSTATIC =	-static
#
# However, sadly, OS X does not support a command-line way of forcing static
# linking.  As a result on OS X if you want to test programs without first
# installing the library in a system directory then it is necessary to set the
# variable DYLD_LIBRARY_PATH in the environment to include the location of the
# library to be tested against.
#
# So, instead of making testing onerous for OS X we use a common trick of giving
# the static library a unique name, such as by appending "_s" or similar to it,
# in order to make it different from the normal name of the dynamic library and
# thus we can trick the linker into thinking there is only a static library
# available.  (This is actually done with a symlink in order to preserve normal
# naming conventions for the actual file.)
#
# Note that the .pc file as-built (for pkgconf, or the older pkg-config, may
# still require adjustments after a "dist" archive is un-packed since it may not
# have been original configured for the final installation directory).
#
# XXX this is, very unfortunately, somewhat ugly and package dependent!
#
# program and test directories need to reference the static library, but we
# cannot just use the portable form of "-l${LIB}_s" because LIB is not always
# set and cannot always be set
#
# see src/Makefile for where LIB is set
#
# So instead we refer to ${PACKAGE}, assuming it has the same name...
#
.if !defined(LIB)
#
# Handling ${.OBJDIR} both with and without use of ${MAKEOBJDIRPREFIX} is
# tedious.
#
. if exists(${.OBJDIR}/${bmake_topdir}/src)
# .OBJDIR is from MAKEOBJDIRPREFIX
LDADD += -L${.OBJDIR}/${bmake_topdir}/src
DPADD = ${.OBJDIR}/${bmake_topdir}/src/lib${PACKAGE}.a
. else
# assume .OBJDIR is a local directory, so look for bmake_topdir from the parent
LDADD += -L../${bmake_topdir}/src/${.OBJDIR:T}
DPADD = ../${bmake_topdir}/src/${.OBJDIR:T}/lib${PACKAGE}.a
. endif
LDADD += -l${PACKAGE}_s
#
.endif	# !defined(LIB)

# NOTICE:  Nothing below here _should_ be project dependent

# yes, always -- we use this because it's safer when using a shared network
# directory for building/testing/etc.  (though it is sometimes somewhat
# annoying)
#
OBJMACHINE =	yes

# N.B.:  ${DESTDIR} if set on the 'install' command line, it is also prepended
# to each of these target directories.
#
INSTALL_OPTS =	# for specific ownerships
# these will be over-ridden by pkgsrc
BINDIR ?= /bin
INCSDIR ?= /include${INCSUBDIR}
LIBDIR ?= /lib
DEBUGDIR ?= /libdata/debug
LINTLIBDIR ?= /libdata/lint
SHAREDIR ?= /share
DOCDIR ?= ${SHAREDIR}/doc
MANDIR ?= ${SHAREDIR}/man

# hmmmm....  This apparently has no precedent in the world of BSD Make.  Base it
# off of LIBDIR....
PKGCONFIGDIR ?= ${LIBDIR}/pkgconfig

# xxx all this otherwise some systems won't allow unprivileged installs
.if ${USER} != "root"
USER_GRP !=	id -g
BINOWN =	${USER}
BINGRP =	${USER_GRP}
LIBOWN =	${USER}
LIBGRP =	${USER_GRP}
SHAREOWN =	${USER}
SHAREGRP =	${USER_GRP}
FILESOWN =	${USER}
FILESGRP =	${USER_GRP}
NOBINMODE =	644		# else OSX bsdmake can't install!
DEBUGOWN =	${USER}
DEBUGGRP =	${USER_GRP}
MANOWN =	${USER}
MANGRP =	${USER_GRP}
DOCOWN =	${USER}
DOCGRP =	${USER_GRP}
# xxx adapt for typo in mk-20150901 through at least mk-20170418
DOGGRP =	${USER_GRP}
.endif

STRIP =		# empty (needed for OSX bsdmake install)

# more hacks to avoid missing system dependencies with DESTDIR set
DPLIBC =		# empty
LIBC =			# empty
LIBCRT0 =		# empty
LIBCRTI =		# empty
LIBCRTBEGIN =		# empty
LIBCRTEND =		# empty
SHLIB_LDSTARTFILE =	# empty
SHLIB_LDENDFILE =	# empty

# Finally some common settings
#
# N.B.:  all sub-directory Makefiles must set ${bmake_topdir} to be a relative
# path to the top of the source tree.  This would not be necessary if/when all
# supported BSDMake versions have the ${.MAKE.LEVEL} feature.  ${.MAKE.LEVEL}
# was first found in NetBSD-current after 2009/09/08 but it is not found in any
# release until 6.0.  Unfortunately there is no easy way to know definitively
# what version of BSDMake is being used.  Simon Gerraty's bmake has
# MAKE_VERSION, as does FreeBSD's make though they are entirely and totally
# different and unrelated.  In NetBSD MAKE_VERSION was only briefly present
# since it represented the build date, not the release date, and it was removed
# in order to make it possible to do reproducible builds.
#
bmake_srcdir =	${.CURDIR}/${bmake_topdir}/src

# This .if is annoying, but some older systems don't support CPPFLAGS.
#
.if defined(CPPFLAGS)
CPPFLAGS +=	-I${bmake_srcdir}
.else
CFLAGS +=	-I${bmake_srcdir}
.endif

#
# Local Variables:
# eval: (make-local-variable 'compile-command)
# compile-command: (concat "MAKEOBJDIRPREFIX=$(pwd)/build " (default-value 'compile-command))
# End:
#
