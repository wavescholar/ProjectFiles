----042314----
Started to read "The C++ Programming Language", 4th Edition [Bjarne Stroustrup].  This is the third time I've read the book.  This version covers C++11, the first major revision since 98.

Blogged on the subject of cleaning up Git repository after a failed commit due to large files size. 

----042414----
Investigated the presence of ANN in SpectralAnalysis.  I have build the library but it is not integrated with the DiffusionMaps package. Mauro relayed to me that the CoverTree implementation had a bug and that is was fixed by one of his students. I am not sure if I have that version or not. 

Built a OpenMP version of SuperLU and blogged about the changes. It is not clear whether this is the best sparse solver. Other choices for a sparse linear solver are UMFPACK and MKL PARDISO. 

Read Chapter 2 "The C++ Programming Language", 4th Edition [Bjarne Stroustrup]. 

Installed Visual Studio 2013 to test new C++11 language features.

Started to write The Beginners Guide To The Bushido Software Development Methodology. 

----042514----
Reorganized all the libraries in the KL source tree.  Cleaned up the organization of code to model best practices for code organization. 

Blogged on the distribution of binaries dependencies of GitHub repositories.  

Blogged on the subject of Intel Compiler crippling performance when running on AMD hardware.  Noted that there are CPUID manipulation routines out there.

I've been thinking about writing practices and word wrapping. Do I want to introduce hard breaks in my writing? GitHub does not word wrap txt. Most modern editors do.  I'm wondering if GitHub will display wrapping if I change the extension of my test files to md.  Update : GitHub wraps.  There is a default line width of 120 characters. The source window or editor wrapped after 120 characters. The window can not be scaled below 120 characters without a scrollbar being introduces.   

----042614---
I worked this day but did not add to my lab book at the time.  CLP Main from 1:00 to 5:00.  It looks like I worked on the build environment.  I did a clean build from git for all projects.  This required some work on the project files. All went well.

----042714----
Rest day.

----042814----
Started to implement Intel VSL wrappers for klVector.  Unit test are being written along the way , but I'm neglecting assert / expect mechanism.  I will integrate these into the Boost unit test framework in May 2014.  Learned how to convert the MKL complex structure internally to the stl complex<double> via a typedef.  These ultimately need to be compatible with the Fortran complex struct.     

----042914----
Finished implementation of all complex<double> varieties of VSL functions and integrated all existing VSL functions into Boost Unit Test project. Also wrote full set of double methods.  

----043014----
Investigating the design choices made in wrapping the VSL functions.  This should have been done in advance as now I will have to go back and adjust the methods.  The considerations are to avoid unnecessary copying of data and that the syntax is as elegant as possible. 

----050114----
Modified all of the klVSL methods to return void and take output parameter as a reference.  

----050214----
Set up the release configuration of boost unit test for klVSL. Started to write the test code for plotting basic VSL functions.  Wrote a performance test for the add function.  Exposed a timing function on the unit test harness. 

----050314----
Wrote a klMemoryManager wrapping the MKL library buffer aligned memory allocation. Added a new base class to klVector that has and instance of this as a global static member. This allows one to optimally have all allocations for klVector on the MKL free store. Speed up for the entire integration test for this was calculated to be 69/61. Wrote a number of GitHub issues again klMatrixCore.

----050414----
Added a benchmark test for the MKL memory manager. Added a benchmark method to the unit test harness that returns timing info.  Moved the test code from MatricCore library to the test projects.  

----050514----
Revived FastGaussTransform.  Worked on getting the Matlab implementation working. 

----050614----
Wrote BLAS based min and max.  Got the Matlab FastGaussTransform working. Organized the source code for FGT.  Wrote point cloud generator for Gaussian Mixture, and uniform hypercube. Tried to get Javascript image gallery working for Octopress but was unsuccessful. 

----050714----
Worked on FGT.  

----050814----
Further work on FGT.  Had problems understanding in versus out parameters.  Started to rework the Latex plotting functions.

----050914----
Completed basic FGT port.  This needs testing and performance benchmarking. Finished reworking the plotting functions. 

----050914----
Wrote binary matrix and vector IO functions.  Speed-up over stream serializers is stunning for the readers. Read about generating random matrices from classical groups and wrote skeleton of blog posting with links.

----051014----
Finished basic binary IO component. Started norm and cond number test.

----051114----
Wrote move constructor.  Cleaning up use of copy ctor in test code. 

----051214----
Cleaning up use of copy ctor in test code. 

----051314----052414----
No work completed

----052514----
Started design of algorithm parameter class.  This is more complicated than it seems.  I'm going to avoid unnecessary copying of data.  We seek a design pattern that allows for extensible use, testing, and serialization. This is going to take a lot of study to do correctly, so we've mocked out a straw man design.

----052614----052814----
Implemented algorithm parameter component.

----053014----060614----
No work completed

----060714----061014----
The algorithm parameter framework was completed and a basic serializer for it was implemented. Considering xml and hdf5 for the future. Read IFGT paper, wonder how relevant the Multipole methods are.  Is there better technology for speeding up Kernel machines?  Also, the metrics for performance may be changing - possibly memory is the limiting variable for some large data sets. Investigate and write up notes for handbook and blog.  Some time was devoted to thinking about how to present content.  By content I mean software, handbook, and blog postings.  There are two lines of development; machine learning and image informatics.  

----061114----
Worked on Aloidia website.  Reset the AWS Route 53 DNS settings. alodia.com was not being resolved. Used the top level name aloidia.com for the A record.  Worked on the integration test, the linear regression needs a meaningful test set.  Use the "sickness atan manifold from Matlab tests. Expanded the integration test interface to take the klTestType parameter.  This will allow for running the test for all the data sizes. Also modified the output to correspond to the test type and whether the Intel memory manager is being used. 

----061314----
Wrote a batch script to check-in multiple Github repositories.  This was a problem to get working until I changed the default git shell to powershell. 
 
----100515----
Completely abandoned all work to pursue Hadoop Spark.  Now doing data analysis at scale.
Returning to GB domain to clean things up and finish handbook. 
  




