## Disclaimer
This software is provided as-is, and does not take any responsibility for bills incurred on the google cloud engine online.

## Credit

This is inspired by ssh-engine by Matt Plays Chess, https://gitlab.com/matt-plays-chess/ssh-engine, a more general-purpose program that works on all remote ssh connections, but require more setup-work.

This work will on itself boot up and shut down google compute engine machines when the engine is loaded and closed, as well as automatically read out how many CPU-cores and how much memory is in the machine, as well as automatically generate ssh-keys for a stable and secure connection.

## Disadvantages

- Hard-to-track pricing scheme. While prices are competitive, it is a bit of work to figure out what you're actually paying. All prices are without VAT.

- Your engine may be kicked off by Google, if customers willing to pay more requests the computational power.

- Ongoing storage costs even if not in use - however at a level of 0.5$/month for a stockfish machine.

- Lots of mails. Every time a Terms & Condition is changed, or a new instance needs to be set up.

- A little bit of effort to set up.

## Advantages

- It's cheap.

- Not bogged down during olympiads.

- No faking of engine performance.

- Ability to setup engine parameters of your choosing.

- Automatic on/off of cloud engine when loaded in chessbase.

## Motivation

Google compute engine is an online service providing virtual machines with rolling billing, with the possibility to get some exceptionally strong computers. This piece of software aims to serve as an intermediate layer between the compute engine virtual machines and chessbase, as a potential replacement or supplement of the chessbase cloud engines.

Spot VMs
--------
Compute engine exposes both regular virtual machines (VMs) and spot VMs, where the spot VMs are machines that run the same way as regular VMs, that can potentially be kicked out, but getting a significant discount. Similar to a system, where a customer that pays more gets to stop in line and put our machine on hold. This does mean that the gcloud_engine.exe program might give an error on engine load, if all virtual machines in the zone are running already, and google are unable to spin up your machine. For now, the error messages are not very informative, and more information can be had by enabling the log.

Pricing
-------
The discounts on spot engines makes the price very competitive compared to chessbase cloud engines. Benchmarks in mid-october for stockfish 17.1 at ~95 MN/s was 0.84$/hour + VAT (/sales tax/MwSt/moms) at the right instance.

The pricing system of the compute engine is based on how many CPU-cores, how much memory, storage and graphics card the VM needs. Storage is a monthly fixed cost, while memory, CPU-cores and graphics card is only while the machine is running. Typically, for Stockfish, it is preferred to have a lot of CPU-performance/CPU-cores, and for Lc0, it is preferred to have a strong graphics card.

Current spot prices: https://cloud.google.com/spot-vms/pricing#section-2

Note that the prices may change up to every 30 days, and there is no straightforward way to recheck the price for the computer apart from manually computing it based on the price list, and the amount of storage, memory and CPUs that the machine uses.

This is not a straightforward or transparent system, however, all the spot-prices on stockfish-instances seem competitive against the CB cloud engines, so even if the price rises, there is a high likelihood that it's still significantly cheaper than the prices on the Chessbase Engine Cloud.

Tested setups
------------------

A few machine configurations have been tested in October 2025:

| Engine            | Virtual Machine | Price    | Performance |
|-------------------|-----------------|----------|-------------|
| Stockfish 17.1    | n2d-highcpu-224 | 0.84$/hr | ~95 MN/s    |
| Lc0 0.32 BT4-1740 | g2-standard-4   | 0.21$/hr | 4 kN/s      |

All prices without VAT/moms/MwSt.

Note that the BT4-1740 is the recommended neural net (NN) on https://lczero.org/play/networks/bestnets/, yet this NN is not represented on the chessbase cloud. Due to its size it will have fewer N/S than the smaller networks used on chessbase cloud.

While a nVidia H100 GPU should perform significantly better, it is not a given that the quota for this GPU is getting approved.

## Prerequisites
Setting up GCloud requires the Google Cloud command line SDK, which can be downloaded here: https://cloud.google.com/sdk/docs/install.

It is recommended to follow the default installation settings all the way through. In the final step of the installation (Screenshot 0), it is not necessary to start the program or add any shortcuts.

## Download release package
The engine gcloud glue layer can be downloaded on the right, under releases. Pick the newest release, download and unzip it.

It will contain the following files:
- gcloud_config.exe: Configuration file for automatic setup of Stockfish and Lc0
- gcloud_engine.exe: The program used for emulating the chess engine.
- .dll-files: Files necessary for the program to run.
- foss_engines.yml: List of engines gcloud_config supports.
- License: Software license.
- Readme: This file.

## Setting up a GCloud virtual machine
There's a few of steps involved in setting up a gcloud VM. All of the steps have their equivalent screenshots in the folder user-guide-screenshots, which supports this text-walkthrough.

- 1: Open the google cloud console on https://console.cloud.google.com/, and click on 'Compute Engine'.

- 2: Click on 'VM instances' in the sidebar. This will give you the list of virtual machines. For now, it should be empty. Click on 'Create instance' in the top.

- 3: You'll now get a list of possible machines. Not all machines are available in all regions. A good starting point could be the N2D-machine type with the preset highcpu. Choose the number of CPU-cores you want. For Lc0, choose a 'GPU'-machine, for example with an nVidia L4 graphics card. Note that Lc0 in this system currently only supports running on one graphics card at a time. Note: An example of a Lc0-machine can be seen in screenshot 17.

- 3.5: If the machine is a big machine, you will have to adjust your 'quota'. This is the upper limit of how big machines you can use. If there's a "You might not have enough quota"-warning in step 3, click the "request quota adjustment". For each quota adjustment request and approval you will get a mail. The adjustment should go through in 1-2 minutes. Note: It is beneficial to wait with this until the final 'zone' is chosen for the VM in step 9.

- 4: In the sidebar, click on 'Data Protection' and choose 'No Backups'. This will reduce costs.

- 5: In the sidebar, click on 'OS and Storage', and click on 'change'. A 10Gb disk is plenty for stockfish, where Lc0 requires ~35 gB. A 'Standard persistent disk' is cheaper in monthly costs.

- 6: In the sidebar, click on 'Observability', and remove the checkmark on 'Install ops agent'.

- 7: In the sidebar, click on 'Advanced', and choose the provisioning model 'Spot'. This will drastically reduce the price.

- 8: Add a checkmark on 'Set a time limit for the VM', and choose how long the upper limit is. This will mean that the engine will disconnect after the given amount of hours, but also limit how much you time you will pay for if you forget the machine overnight, or if there's any software-bugs in this tool meaning the machine doesn't turn down gracefully, or in the case of a computer-crash. While optional, this is highly recommended.

- 9: Pick from the different regions in machine configuration. Choose somewhere cheap. A full pricelist for spot engines can be seen at https://cloud.google.com/spot-vms/pricing?hl=en, but requires manual calculation. The easiest guide is the hourly price approximation in the top right corner. Once you've found a region, click on 'create' in the bottom. Now, the machine will be slowly spun up and is ready to go.

- 10: You may have to adjust the quota before it's ready to launch. On adjusting the quota: See step 3.5.

## Using gcloud_config.exe for local configuration

- 11: Double-click on gcloud_config.exe.

- 12: Pick the engine to set up, and enter your instance name and zone as they're listed on the VM Instances page seen in step 2.

- 13: The program will automatically prepare a folder having the same name as the instance name. Note that this may take a few minutes for stockfish, and up half an hour for Lc0.

- 14: The folder is created, and gcloud_engine.exe, the .dll-files and an engine.yml is created in there.

- 15: In Chessbase, click 'Create new UCI engine', and choose gcloud_engine.exe in the child-folder from step 12 and 13. You're now ready to go.

## Analysis with gcloud_engine.exe

The inbetween-layer will automatically start and stop your virtual machine in the google cloud, such that there is no need for opening the website every time you start the engine. However, the engines start-time also means that once the engine is loaded, the field where the analysis lines show up will be empty for roughly a minute, as the cloud computer is booting.

This layer will also ensure that the cloud machine is automatically turned off when you unload the engine in a chessbase window, to minimize hassle of use. Unloading the engine sends the shutdown-signal, but it does not wait for shutdown to complete, so reloading the engine immediately afterwards may cause some mild issues. It may also take up to a minute for the website to indicate again that the engine is turned off.

- 16: Virtual machine being turned off in the gcloud website after use.

## engine.yml file

The engine.yml file next to the engine is what contains the settings. However, it is also possible to toggle a few settings that are not on by default:

- Logging: Setting logging to 'enabled' will create a file log.txt while the engine is being run, that traces all communication with the remote machine and chessbase. This is very useful for debugging, but the file will grow quickly of left on normally.

- hash: It is possible to manually set the hash size of the cloud computer. However, this is best left unset, as the program automatically reads how much RAM the cloud computer has, and sets the hash accordingly while leaving up to 8Gb for the operating system.

- cpus: Same as hash. Best left untouched.

## For developers

Building
------------

Building for windows requires ninja, a regular cmake install not using Visual Studio and the MSVC compilers installed already. The commands for building are as follows executed from the visual studio developer command prompt. Note: When installing Visual Studio insiders C++ tools for MSVC, it comes bundled with Visual Studio CMake. This may conflict with the regular CMAKE in the path, and give errors indicating that it cannot find Ninja. To fix, remove CMake using the visual studio installer.

- cmake --preset=vcpkg
- cmake --build build

This will leave the .dll-files and .exe-files in a separate build-folder.

Vcpkg is included as a submodule, and does therefore not require any installation. Vcpkg will automatically download the dependencies to ssh, crypto and yaml libraries.

Making a release
----------------

Making a release is as simple as pushing a tag starting with 'v' to github, and a release will be automatically generated using a github action.
