## Copyright (C) 2014 Andreas Weber <andy.weber.aw@gmail.com>
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
## this program; if not, see <http:##www.gnu.org/licenses/>.

## Run all tests

run ('../PKG_ADD')
addpath('../inst')

fns = { '../src/__v4l2_handler__.cc';
        '@videoinput/videoinput';
        '@videoinput/get';
        '@videoinput/set';
        '@videoinput/getsnapshot';
        'imaqhwinfo'};
len = numel(fns);
f = fopen ('fntest.log', 'w');
n = m = zeros(len, 1);
for i=1:len
  [n(i), m(i)] = test(fns{i}, 'quiet', f);
  if (n(i) == m(i))
    j = '';
  else
    j = 'FAIL';
  endif
  printf ('%s%s PASS %d/%d %s\n', fns{i}, repmat('.', 1, 50-numel(fns{i})), n(i), m(i), j);
endfor
printf ("\nSummary:\n\n");
printf ("  PASS  %2d\n", sum(n));
printf ("  FAIL  %2d\n\n", sum(m)-sum(n));
fclose(f);

