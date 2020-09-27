% Assumes the data dir function returns either the location where
% getrun exists or a subdirectory one level down.
pdir = ne_load_runsdir('RFD_DATA_DIR');
cd(pdir);
[fd,msg] = fopen('runs.dat','r');
if fd <= 0
  [fd,msg] = fopen('../runs.dat','r');
  if fd > 0
    cd ..
  end
end
if fd > 0
    tline = fgetl(fd);
    while ischar(tline)
        fprintf(1,'Processing: "%s"\n', tline);
        if exist(tline,'dir') == 7
            oldfolder = cd(tline);
            csv2mat;
            delete *.csv
            cd(oldfolder);
        end
        tline = fgetl(fd);
    end
    fclose(fd);
    delete runs.dat
end
ui_rfd;
