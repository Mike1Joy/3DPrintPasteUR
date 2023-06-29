from os import listdir
from os.path import isfile, join
from os import getcwd

filename = "run"
fileext = ".nc"
empty_file_cont = "M62 P0 M62 P1\n"
num_lines = 40000
num_files = 5

def GetValueInt(str, line):
    return round(float(line.split(str,maxsplit=1)[-1].split(maxsplit=1)[0]))

def GetValueFloat(str, line):
    return float(line.split(str,maxsplit=1)[-1].split(maxsplit=1)[0])

def main():
    dir_files = [f for f in listdir(getcwd()) if isfile(join(getcwd(), f)) and ".gcode" in f]
    if (len(dir_files) == 0):
        print('No GCode file found. Make sure the file extension is ".gcode"')
        return
    if (len(dir_files) > 1):
        print(f'{len(dir_files)} GCode files found:')
        for i,f in enumerate(dir_files):
            print(f"  {i+1}: {f}")
        gcode_file = dir_files[int(input("Enter the number of the file to split: "))-1]
    else:
        gcode_file = dir_files[0]

    with open(gcode_file, "r") as f:
        lines = f.readlines()
        print(f'"{gcode_file}" has {len(lines)} lines of GCode.')

        if len(lines) > num_lines*num_files:
            print(f"Warning:")
            print(f"    Too many lines to be split into {num_files} files with {num_lines} lines each.")
            print(f"    Max number of lines is {num_lines*num_files}. Increase the number to files both in this python script and on the robot program to run this GCode")
        else:
            print(f"File will be split into {num_files} files...")

    G0_F = 999
    G1_F = 999
    X = 0
    Y = 0
    Z = 0

    files = []
    for file_num in range(num_files):
        files.append(open(filename+str(file_num+1)+fileext,"w"))

    file_num = 1
    print(f'Writting "{filename+str(file_num)+fileext}"...')

    # Write lines
    for n,l in enumerate(lines):
        if n > file_num*num_lines:
            file_num += 1
            print(f'Writting "{filename+str(file_num)+fileext}"...')
            files[file_num-1].write(f"G0 F{G0_F} X{X} Y{Y} Z{Z}\n")
            files[file_num-1].write(f"G1 F{G1_F} X{X} Y{Y} Z{Z}\n")
        
        if "F" in l:
            if "G0" in l:
                G0_F = GetValueInt("F",l)
            elif "G1" in l:
                G1_F = GetValueInt("F",l)
        
        if "G0" in l or "G1" in l:
            if "X" in l:
                X = GetValueFloat("X",l)
            if "Y" in l:
                Y = GetValueFloat("Y",l)
            if "Z" in l:
                Z = GetValueFloat("Z",l)

        files[file_num-1].write(l)
    
    # Deal with empty files
    for n in range(file_num, num_files):
        print(f'Writting "{filename+str(n+1)+fileext}" (empty)...')
        files[n].write(";EMPTY FILE\n")
        files[n].write(f"G0 F{G0_F} X{X} Y{Y} Z{Z}\n")
        files[n].write(f"G1 F{G1_F} X{X} Y{Y} Z{Z}\n")
        files[n].write(empty_file_cont)

    for f in files:
        f.close()

    print(f"File successfully split into {num_files} files.")
        
if __name__ == "__main__":
    main()
    print()
    input("Press enter to exit...")
