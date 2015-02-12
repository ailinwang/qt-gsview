
function Component()
{
    //  default constructor
}

Component.prototype.createOperations = function()
{
    //  call default implementation to actually install something
    component.createOperations();

    //  set up desktop file and a link on the desktop
    if (installer.value("os") === "x11")
    {
        component.addOperation("CreateDesktopEntry", "@TargetDir@/gsview.desktop","Version=6.0\nType=Application\nTerminal=false\nExec=@TargetDir@/gsview\nName=GSView 6.0\nIcon=@TargetDir@/gsview_app.png\nName[en_US]=GSView 6.0");

        component.addOperation("Copy", "@TargetDir@/gsview.desktop", "@HomeDir@/Desktop/gsview.desktop");
    }

}
