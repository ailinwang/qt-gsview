
function Component()
{
    // default constructor
    component.loaded.connect(this, this.installerLoaded);
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install something
    component.createOperations();
}

Component.prototype.installerLoaded = function()
{
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory);

    targetDirectoryPage = gui.pageWidgetByObjectName("DynamicTargetWidget");
    targetDirectoryPage.windowTitle = "Choose Installation Directory";
    targetDirectoryPage.description.setText("Please select where "+installer.value("Name")+" will be installed:");
    targetDirectoryPage.targetDirectory.textChanged.connect(this, this.targetDirectoryChanged);
    targetDirectoryPage.targetDirectory.setText(installer.value("TargetDir"));
    targetDirectoryPage.targetChooser.released.connect(this, this.targetChooserClicked);

    gui.pageById(QInstaller.PerformInstallation).entered.connect(this, this.componentPerformInstallationPageEntered);
}

var lastTargetDir = "";

Component.prototype.targetChooserClicked = function()
{
    lastTargetDir = targetDirectoryPage.targetDirectory.text;
    var dir = QFileDialog.getExistingDirectory("", targetDirectoryPage.targetDirectory.text);
    targetDirectoryPage.targetDirectory.setText(dir);
}

Component.prototype.targetDirectoryChanged = function()
{
    var dir = targetDirectoryPage.targetDirectory.text;
    if (dir != "")
    {
		if (installer.fileExists(dir) && installer.fileExists(dir + "/Uninstaller.app"))
		{
			//  update
			targetDirectoryPage.warning.setText("<p style=\"color: green\">You're updating an existing installation.</p>");
		}
		else if (installer.fileExists(dir))
		{
			//  overwrite - warn
			targetDirectoryPage.warning.setText("<p style=\"color: red\">You're installing into an existing directory. You may not want to do this.</p>");
		}
		else
		{
			//  clean
			targetDirectoryPage.warning.setText("");
		}
		installer.setValue("TargetDir", dir);
    }
    else
    {
    	//  user cancelled chooser, restore previous value
		installer.setValue("TargetDir", lastTargetDir);
		targetDirectoryPage.targetDirectory.setText(installer.value("TargetDir"));
    }
}

Component.prototype.componentPerformInstallationPageEntered = function ()
{
	//  before we install, we'll run the uninstaller silently, if we find one.
    var dir = installer.value("TargetDir");
    if (installer.fileExists(dir) && installer.fileExists(dir + "/Uninstaller.app"))
    {
        installer.execute(dir + "/Uninstaller.app/Contents/MacOS/Uninstaller", "--script=" + dir + "/auto_uninstall.qs");
	}
}
