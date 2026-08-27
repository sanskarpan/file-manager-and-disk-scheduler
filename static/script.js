function listFiles() {
    fetch('/list-files')
        .then(response => response.json())
        .then(files => {
            document.getElementById('output').innerHTML = `<pre>${files.join('\n')}</pre>`;
        })
        .catch(error => console.error('Error listing files:', error));
}

function createFile() {
    const fileName = prompt('Enter the name of the file to create (including extension):');
    if (fileName) {
        fetch('/create-file', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fileName })
        })
            .then(response => response.text())
            .then(message => {
                document.getElementById('output').innerHTML = `<pre>${message}</pre>`;
            })
            .catch(error => console.error('Error creating file:', error));
    }
}

function deleteFile() {
    const fileName = prompt('Enter the name of the file to delete (including extension):');
    if (fileName) {
        fetch('/delete-file', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fileName })
        })
            .then(response => response.text())
            .then(message => {
                document.getElementById('output').innerHTML = `<pre>${message}</pre>`;
            })
            .catch(error => console.error('Error deleting file:', error));
    }
}

function renameFile() {
    const oldFileName = prompt('Enter the current name of the file (including extension):');
    const newFileName = prompt('Enter the new name of the file (including extension):');
    if (oldFileName && newFileName) {
        fetch('/rename-file', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ oldFileName, newFileName })
        })
            .then(response => response.text())
            .then(message => {
                document.getElementById('output').innerHTML = `<pre>${message}</pre>`;
            })
            .catch(error => console.error('Error renaming file:', error));
    }
}

function editFile() {
    const fileName = prompt('Enter the name of the file to edit (including extension):');
    if (fileName) {
        fetch(`/view-file?fileName=${encodeURIComponent(fileName)}`)
            .then(response => response.text())
            .then(content => {
                const newContent = prompt('Edit file content:', content);
                if (newContent !== null) {
                    fetch('/edit-file', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ fileName, content: newContent })
                    })
                        .then(response => response.text())
                        .then(message => {
                            document.getElementById('output').innerHTML = `<pre>${message}</pre>`;
                        })
                        .catch(error => console.error('Error editing file:', error));
                }
            })
            .catch(error => console.error('Error fetching file content:', error));
    }
}

function searchFile() {
    const fileName = prompt('Enter the name of the file to search for (including extension):');
    if (fileName) {
        fetch('/search-file', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fileName })
        })
            .then(response => response.json())
            .then(result => {
                const message = result.found ? `File found: ${result.fileName}` : 'File not found';
                document.getElementById('output').innerHTML = `<pre>${message}</pre>`;
            })
            .catch(error => console.error('Error searching for file:', error));
    }
}

function fileDetails() {
    const fileName = prompt('Enter the name of the file to get details (including extension):');
    if (fileName) {
        fetch('/file-details', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fileName })
        })
            .then(response => response.json())
            .then(details => {
                document.getElementById('output').innerHTML = `<pre>${JSON.stringify(details, null, 2)}</pre>`;
            })
            .catch(error => console.error('Error getting file details:', error));
    }
}

function viewFile() {
    const fileName = prompt('Enter the name of the file to view (including extension):');
    if (fileName) {
        fetch(`/view-file?fileName=${encodeURIComponent(fileName)}`)
            .then(response => response.text())
            .then(content => {
                document.getElementById('output').innerHTML = `<pre>${content}</pre>`;
            })
            .catch(error => console.error('Error viewing file:', error));
    }
}

function sortFileContents() {
    const fileName = prompt('Enter the name of the file to sort contents (including extension):');
    if (fileName) {
        fetch('/sort-file-contents', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fileName })
        })
            .then(response => response.text())
            .then(sortedContent => {
                document.getElementById('output').innerHTML = `<pre>${sortedContent}</pre>`;
            })
            .catch(error => console.error('Error sorting file contents:', error));
    }
}

function listDirectories() {
    fetch('/list-directories')
        .then(response => response.json())
        .then(directories => {
            document.getElementById('output').innerHTML = `<pre>${directories.join('\n')}</pre>`;
        })
        .catch(error => console.error('Error listing directories:', error));
}
