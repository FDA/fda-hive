# fdahive

fdahive is a Python module for interacting with HIVE instances. It enables users to login, download objects, retrieve information about objects, and upload files.
fdahive can be used either as a module within Python or as a command-line tool invoked with `fdahivecli`.

## Dependencies

fdahive requires Python >= 3.6 and the [requests library](https://requests.readthedocs.io/en/master/) >= 2.9.2.

Building the pip package will require the latest version of pip and PyPA's build tool called build.

## Buidling and installing fdahive

fdahive can be built as a pip package. To build the pip package, first make sure the latest version of the build tool is installed:

```
python3 -m pip install --upgrade build
```

and then run the build command in the top-level directory

```
python3 -m build
```

A package will be built in the `dist` folder. It can installed by passing the package to pip

```
python3 -m pip install dist/fdahivecli-1.0.0.tar.gz
```

Users who do not want to use pip can add the source directory to Python's module search path using PYTHONPATH or another preferred method.

## Using the library

Usage of the fdahive library revolves arounds its `HiveSession` class. Users initialize a `HiveSession` object for a given HIVE instance and use the object to login, download files,
upload files, retreive information about HIVE objects, and logout.

Files and computations in HIVE are stored as objects, each assigned a unique ID. This library represents accepts these IDs as either integers or strings.

### Example usage

```
import fdahive
with fdahive.HiveSession('https://dnahive.fda.gov') as hs:
     hs.login()
     hs.download([1236, 4456, 4457])
     hs.upload(['file1.fasta', 'file2.fasta'])
     obj_list = hs.get_reads()
```

### Class method overview

- `HiveSession`
    - `login`
    - `logout`
    - `download`
    - `upload`
    - `get_all_objs`
    - `get_folders`
    - `get_genomes`
    - `get_reads`
    - `get_files`
    - `get_computations`
  
### More detailed usage

#### Initialization

```
HiveSession(url, ssl_verify=True)
```

`HiveSession` objects are intialized by provided the url of the HIVE instance the user wishes to interact with. To interact with the HIVE instance
at dnahive.fda.gov, initialize a `HiveSession` object as follows
```
import fdahive
hs = fdahive.HiveSession('https://dnahive.fda.gov')
```
Advanced users can also specify SSL verification options through the `ssl_verify` arguments. While the majority of users should not modify this argument, a path to
proper SSL certificates can be supplied if Python fails to find them
```
import fdahive
hs = fdahive.HiveSession('https://dnahive.fda.gov', ssl_verify='path/to/certificates')
```
`ssl_verify` can be set to `False` to disable the checking of the certificates, although this is not recommended. See the [requests documentation](https://docs.python-requests.org/en/latest/api/#requests.Session.request) of the `verify` parameter for more information.

The `HiveSession` class supports Python's `with` syntax so that session will log out automatically when the block exits. Users are encouraged to use this
`with` syntax to avoid staying logged in.

```
with fdahive.HiveSession('https://dnahive.fda.gov') as hs:
     hs.login()
     hs.download([1234])
print('Hello World')
>>> Login successful
>>> Downloading object 1234...
>>> Logout successful
>>> Hello World
```


#### Logging in

```
HiveSession.login(username=None, token=None, cred_file='~/.hive-creds', save_creds=False)
```

The `HiveSession` class provides a `login` method that allows users to login with username and token passed in as a parameter, from a credentials file, or console input, in that order of priority. To login with the credentials passed through parameters:
```
import fdahive
hs = fdahive.HiveSession('https://dnahive.fda.gov')
hs.login('User.Name@email.com', 'token')
```
If those are not provided, the login function will search for a credentials file at ~/.hive-creds by default. The file is expected to contain lines of the format https://username:token@hostname.
`login` will choose the first username-token pair with the same hostname as the url of the HIVE instance given at the HiveSession construction.
Alternative paths can be provided through the `cred_file` parameters. The credentials file must have file permissions of 600 or `login` will refuse to use it.
An example credentials file might look like the following:
```
https://User.Name@email.com:token1@dnahive.fda.gov
https://User.Name@email.com:token2@hive.biochemistry.gwu.edu
```        
The user can then log in to dnahive.fda.gov with the username User.Name@email.com and the token token1 by
```
import fdahive
hs = HiveSession('https://dnahive.fda.gov')
hs.login()
```
Finally, if the username and token are not provided through either the method parameters or the credentials file `login` will prompt the user for them as console input.
Tokens hardcoded into scripts are a security weakness. Users are recommended to either use the `login` credentials file, input prompts, or use Python's built-in `getpass`
module.

Set `save_creds=True` to append the supplied credentials to the credentials file on successful login. This will create the credentials file if it does not exist.

#### Downloading objects

```
HiveSession.download(objs, dst_dir=None, overwrite=False, make_obj_dir=False, heptagon_csv_profile=False)
```

Objects can be downloaded through the `download` method. The method takes a list of HIVE object IDs as integers and downloads them to the current working directory
```
import fdahive
with fdahive.HiveSession('https://dnahive.fda.gov') as hs:
     hs.login()
     hs.download([1235, 2223])
```
Objects can be downloaded to another directory by specifying the `dst_dir` option. By default the `download` method will not ovrwrite existing files. To overwrite existing files
set `overwrite=True`. Finally, directories can be created for each object by setting `make_obj_dir=True`. For example,
`download([100, 101, 102], 'dst_dir', make_obj_dir=True)` will create three directories: 'dst_dir/100', 'dst_dir/101', and 'dst_dir/102'

After finishing the method will return a list of paths of successfully downloaded files.

`heptagon_csv_profile` is an option that pertains to downloading HIVE Heptagon objects. By default, Heptagon objects are downloaded as an industry-standard VCF. However, advanced users who want access to HIVE's custom CSV format for profiles can set `heptagon_csv_profile=True`. The default `heptagon_csv_profile=False` is recommended for typical usage.

#### Uploading files

```
HiveSession.upload(self, pathnames, max_threads=None, folder=None, reference=None, index=False, qc=False, screen=False)
```

The `upload` method takes a list of file paths to upload. Directories passed to this method will be uploaded recursively with their file structure intact. On completion the
object ID of the archiver proccess for processing the uploaded files will be printed.

The following example illustrates some of the methods optional parameters
```
import fdahive
with fdahive.HiveSession('https://dnahive.fda.gov') as hs:
     hs.login()
     hs.upload(['file1.fasta', 'file2.fasta', 'path/to/dir'], max_threads=4, folder=1426, qc=True)
```
`max_threads` configures the maximum number of threads that will be executed concurrently. The number of concurrent threads depends partly on the response from the HIVE instance, so
there is no guarantee that the maximum provided here will be used. The object ID of the destination folder can be provided through the `folder` argument. The `index`, `qc`, and
`screen` arguments determine whether to run indexing, quality control, and screening on the uploaded files.

When uploading alignments, the object ID of the reference associated with the alignment can be provided through the `reference` argument.

#### Retreiving object information

```
HiveSession.get_all_objs(type='', print_table=True)
```

Information about existing objects in a HIVE instance can be retreived using the `get_*` methods. `get_all_objs()` will return information on all the objects in a HIVE system
the user has permissions to see. Specific types of objects, such as folders, reads, genomes, etc. can be retreived with the other `get_*` methods. The `print_table` argument
controls whether the object information will be printed to output or returned as a list

#### Logging out

```
HiveSession.logout()
```

Once finished the user can log out of a session through the `logout` method. The method will clear cookie data even if the network requests fail to send.

The `logout` function will be called automatically if the `with` syntax is used with `HiveSession`. Users are encouraged to do this to avoid staying logged in.

## Using the command-line tool

The command-line tool can be invoked using `fdahivecli` if instaled through pip or by executing the module as a script with `python3 -m fdahive`. Help can be obtained by
passing the help command to `fdahivecli` and each of it's three subcommands

```
fdahivecli --help
fdahivecli down --help
fdahivecli up --help
fdahiveclu view --help
```