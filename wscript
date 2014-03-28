#! /usr/bin/env python

APPNAME = 'librecontext'
VERSION = '0.1'

top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    conf.check_cfg(package='redland', args='--cflags --libs')

def build(bld):
    bld.recurse('src')
    bld.recurse('tests')
