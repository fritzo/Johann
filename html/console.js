
//text tools
function format (s)
{
    return s.replace(/#.*/g , " ") //uncomment
            .replace(/^\s+/g , "") //strip leading space
            .replace(/\s+$/g , "") //strip trailing space
            .replace(/\s+/g , " ") //smash remaining space
}

//command wrappers
function Run (command)
{
    document.form.command.value = command
    document.form.input.value = document.form.editor.value //echo
    document.form.submit()
}
function File (command)
{
    document.form.command.value = command
    document.form.input.value = document.form.file.value
    document.form.submit()
}
function Send (command)
{
    var code = format(document.form.editor.value)

    //check non-emptiness
    if (!code) {
        alert("type an expression in the orange window")
        document.editor.input.focus()
        return
    }

    //send command to server
    document.form.command.value = command
    document.form.input.value = code
    document.form.submit()
}

