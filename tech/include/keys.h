///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_KEYS_H
#define INCLUDED_KEYS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

enum eMouseStateFlags
{
   kLMouseDown = (1<<0),
   kMMouseDown = (1<<1),
   kRMouseDown = (1<<2)
};

enum eKeyCode
{
   kTab              = 9,
   kEnter            = 13,
   kSpace            = 32,
   kEscape           = 27,

   kBASE = 128,
   kF1               = (kBASE + 1),
   kF2               = (kBASE + 2),
   kF3               = (kBASE + 3),
   kF4               = (kBASE + 4),
   kF5               = (kBASE + 5),
   kF6               = (kBASE + 6),
   kF7               = (kBASE + 7),
   kF8               = (kBASE + 8),
   kF9               = (kBASE + 9),
   kF10              = (kBASE + 10),
   kF11              = (kBASE + 11),
   kF12              = (kBASE + 12),
   kInsert           = (kBASE + 13),
   kDelete           = (kBASE + 14),
   kHome             = (kBASE + 15),
   kEnd              = (kBASE + 16),
   kPageUp           = (kBASE + 17),
   kPageDown         = (kBASE + 18),
   kBackspace        = (kBASE + 20),
   kCtrl             = (kBASE + 22),
   kLShift           = (kBASE + 23),
   kRShift           = (kBASE + 24),
   kAlt              = (kBASE + 25),
   kUp               = (kBASE + 26),
   kDown             = (kBASE + 27),
   kLeft             = (kBASE + 28),
   kRight            = (kBASE + 29),
   kPause            = (kBASE + 30),
   kMouseLeft        = (kBASE + 31),
   kMouseMiddle      = (kBASE + 32),
   kMouseRight       = (kBASE + 33),
   kMouseWheelUp     = (kBASE + 34),
   kMouseWheelDown   = (kBASE + 35),
   kMouseMove        = (kBASE + 36),
};

///////////////////////////////////////////////////////////////////////////////
// does the given key code actually represent a mouse action?

inline bool KeyIsMouse(int key)
{
   if (key == kMouseMove ||
       key == kMouseLeft ||
       key == kMouseMiddle ||
       key == kMouseRight ||
       key == kMouseWheelUp ||
       key == kMouseWheelDown)
   {
      return true;
   }
   else
   {
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_KEYS_H
