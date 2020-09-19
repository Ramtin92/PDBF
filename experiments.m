clc;
close all;
clear ;

%read in an image

%path = 'C:/Users/shosse02/Desktop/TD_RGB_A_Set1/10';   
%path = 'C:/Users/shosse02/Desktop/tufts_face_database/TD_RGB_Curated_Dataset_45';
%path = 'C:/Users/shosse02/Desktop/Forensic_Sketch/Forensic_Sketch';
path = 'C:/Users/shosse02/Desktop/TD_RGB_Curated_Dataset_45';

fileFolder=fullfile(path);
dirOutput=dir(fullfile(fileFolder,'*.jpg'));
fileNames={dirOutput.name};
 for index = 1:53
    filename = strcat(path, '/', string(fileNames(index)));
    fprintf(filename);
    I = imread(filename);
    I = rgb2gray(I);
    imwrite(I, strcat(path, '/', sprintf('img_%d.jpg', index)));
end
for decomposition_method = 0:1
    for numberBitplanes = 1:8
        sub_dir = strcat(path, '/', sprintf("decomp_%d_np_%d", decomposition_method, numberBitplanes));
        mkdir(sub_dir);
        sub_dir_recon = strcat(path, '/', sprintf("decomp_%d_np_%d_recon", decomposition_method, numberBitplanes));
        mkdir(sub_dir_recon);
        for index = 1:53
            filename = strcat(path, '/', string(fileNames(index)));
            fprintf(filename);
            I = imread(filename);
            I = rgb2gray(I);

            %------------------parameters-------------------%
            %numberBitplanes = 3;     % chose the number of bitplanes
            %decomposition_method = 0; % 0 for Bit plane decomposition method and 
                                       % 1 for fibonnacci decomposition method  

            p_code = 2;             % p-code for generating fib sequence
            n_code = 16;            % n-code for generating fib sequence

            beta = 0.0;             % beta for
            winsize = 2;            % winsize for PDBF

            gausian_sigma = 1;      % gaussian smoothing sigma
            gausian_kernel = 5;     % gaussian smoothing filter size
            gausian_employ = 1;     % 0 for not using gausian smoothing, 1 for using
            %------------------parameters-------------------%

            %now make a direct call to the BALogix algorigthm
            [J ,inImage_struc, outImage_struc, reconImage_struc] =  BALogix_Interface(I,...
                numberBitplanes, decomposition_method, p_code, n_code, beta, ...
                winsize, gausian_sigma, gausian_kernel, gausian_employ);

            outImage = uint8(outImage_struc.scan02)*255;
            ReconImage = uint8(reconImage_struc.scan03);
            output_f_name = strcat(sub_dir, '/', sprintf('%d.jpg', index));
            
            output_recon_f_name = strcat(sub_dir_recon, '/', sprintf('%d.jpg', index));
%             strcat(path, '/', sprintf("img_%d_decomp_%d_np_%d", index, decomposition_method, numberBitplanes), '.jpg');
            imwrite(outImage ,output_f_name);
            imwrite(ReconImage, output_recon_f_name);
        end
    end
end
