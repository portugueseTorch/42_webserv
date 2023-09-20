window.addEventListener('load', toDel);

function toDel() {
    const elems = document.getElementsByClassName("toDelete");

    // console.log(elems);
    // console.log(elems.length);

    for (i = 0; i < elems.length; i++) {
        elems[i].addEventListener('click', deleteItem);
    }
}

const deleteItem = (event) => {
    const name = event.target.parentNode.children[0].innerHTML;
    console.log(name);
    fetch('delete.py', {
        method: 'DELETE',
        body: 'name=' + name
    })
}

