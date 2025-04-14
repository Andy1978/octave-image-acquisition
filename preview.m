## Copyright (C) 2025 Andreas Weber <andy.weber.aw@gmail.com>
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 3 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, see <http://www.gnu.org/licenses/>.

close all
clear h

pkg load image-acquisition

graphics_toolkit qt

h.ax_img  = axes ("position", [0.05 0.55 0.5 0.4]);
h.ax_hist = axes ("position", [0.05 0.05 0.5 0.4]);
h.fcn = @(x) polyval([-0.1 0.5 3 0], x);
h.vi = videoinput ();
#start (h.vi);

function update_preview (obj, init = false)

  ## gcbo holds the handle of the control
  h = guidata (obj);
  #replot = false;
  #recalc = false;
  %~ switch (gcbo)
    %~ case {h.print_pushbutton}
      %~ fn =  uiputfile ("*.png");
      %~ print (fn);
    %~ case {h.grid_checkbox}
      %~ v = get (gcbo, "value");
      %~ grid (merge (v, "on", "off"));
    %~ case {h.minor_grid_toggle}
      %~ v = get (gcbo, "value");
      %~ grid ("minor", merge (v, "on", "off"));
    %~ case {h.plot_title_edit}
      %~ v = get (gcbo, "string");
      %~ set (get (h.ax, "title"), "string", v);
    %~ case {h.linecolor_radio_blue}
      %~ set (h.linecolor_radio_red, "value", 0);
      %~ replot = true;
    %~ case {h.linecolor_radio_red}
      %~ set (h.linecolor_radio_blue, "value", 0);
      %~ replot = true;
    %~ #case {h.linestyle_popup, h.markerstyle_list}
    %~ #  replot = true;
    %~ case {h.noise_slider}
      %~ recalc = true;
  %~ endswitch

  #if (recalc || init)
    #img = getsnapshot (h.vi);
    img = randi (400,300,3);
    #set (h.noise_label, "string", sprintf ("Noise: %.1f%%", noise * 100));
    if (init)
      h.img = image (h.ax_img, img);
      h.hist = plot (h.ax_hist, 0,0);
      guidata (obj, h);
    else
      set (h.img, "cdata", img);
      # calculate histogram
      n_red = histc(img(:,:,1)(:), 0:10:255);
      set (h.hist, "ydata", n_red);
    endif
  #endif

  #if (replot)
    #cb_red = get (h.linecolor_radio_red, "value");
    #lstyle = get (h.linestyle_popup, "string"){get (h.linestyle_popup, "value")};
    #lstyle = strtrim (lstyle(1:2));
    #mstyle = get (h.markerstyle_list, "string"){get (h.markerstyle_list, "value")};
    #if (strfind (mstyle, "none"))
    #  mstyle = "none";
    #else
    #  mstyle = mstyle(2);
    #endif
    #set (h.plot, "color", merge (cb_red, [1 0 0 ], [0 0 1]),
    #             "linestyle", lstyle,
    #             "marker", mstyle);
    
    
    # Update possible VideoFormat list
    set (h.videoformat_popup, "string", {set(h.vi, "VideoFormat").fourcc});
    
    # Update VideoResolution list
    res = set(h.vi, "VideoResolution");
    res_str = strsplit(sprintf ("%i x %i px;", res.')(1:end-1), ";");
    set (h.videoresolution_popup, "string", res_str);
    
    # Update VideoFrameInterval list
    fi = set(h.vi, "VideoFrameInterval");
    fi_str = strsplit(sprintf ("%i/%i s;", fi.')(1:end-1), ";");
    set (h.videoframeinterval_popup, "string", fi_str);
    
    keyboard
    
    
    
  #endif
  
endfunction

# VideoFormat
h.videoformat_label = uicontrol ("style", "text",
                                 "units", "normalized",
                                 "string", "VideoFormat:",
                                 "horizontalalignment", "left",
                                 "position", [0.57 0.85 0.2 0.08]);

h.videoformat_popup = uicontrol ("style", "popupmenu",
                                 "units", "normalized",
                                 "callback", @update_preview,
                                 "position", [0.8 0.85 0.2 0.08]);

# VideoResolution
h.videoresolution_label = uicontrol ("style", "text",
                                     "units", "normalized",
                                     "string", "VideoResolution:",
                                     "horizontalalignment", "left",
                                     "position", [0.57 0.75 0.2 0.08]);

h.videoresolution_popup = uicontrol ("style", "popupmenu",
                                     "units", "normalized",
                                     "callback", @update_preview,
                                     "position", [0.8 0.75 0.2 0.08]);

# VideoFrameInterval
h.videoframeinterval_label = uicontrol ("style", "text",
                                        "units", "normalized",
                                        "string", "VideoFrameInterval:",
                                        "horizontalalignment", "left",
                                        "position", [0.57 0.65 0.2 0.08]);

h.videoframeinterval_popup = uicontrol ("style", "popupmenu",
                                        "units", "normalized",
                                        "callback", @update_preview,
                                        "position", [0.8 0.65 0.2 0.08]);


set (gcf, "color", get(0, "defaultuicontrolbackgroundcolor"))
guidata (gcf, h)
update_preview (gcf, true);

pause (2)
