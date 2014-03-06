The official repository for Downpour, a WYSIWYG drop-in level editor for Rayne. It contains everything required to build the Downpour module itself. Forks are welcome!

## Building

**Note:** As of now, Downpour may use API that is not yet available in the latest Rayne alpha release, making compilation impossible. All used API will be available eventually, however, in the meantime, please don't open tickets about missing but required API.

 * Open the Downpour project in your IDE and build it.
 * Create a new folder named `Downpour` and move the build `dylib` or `dll` into it
 * Copy the `Downpour/Resources` into the newly created folder as well
 * Move the `Downpour` folder into your projects module directory and change the `manifest.json` to also load the `libDownpour` module
 
## Opening Downpour

By default you can launch Downpour by either pressing `F11` on your keyboard, or alternatively by sending `DPToggle` via the `MessageCenter` class. Example:

	void MyWorld::LoadOnThread(RN::Thread *thread)
	{
	   // ... Actual loading here
	   
	   // Activate Downpoour
	   RN::MessageCenter::GetSharedInstance()->PostMessage(RNCSTR("DPToggle"), nullptr, nullptr);
	}
	
## License

The source code is available under the MIT license, additional resources may be available under a different license. See the `LICENSE.md` file for mor information.