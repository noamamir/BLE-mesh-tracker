import {Component, Input} from '@angular/core';
import {ReceiverAliasService} from "../../services/receiverAlias/receiver-alias.service";

@Component({
  selector: 'app-alias-name-input',
  templateUrl: './alias-name-input.component.html',
  styleUrls: ['./alias-name-input.component.css']
})
export class AliasNameInputComponent {
  @Input() originalName: string
  constructor(
    public receiverAlias: ReceiverAliasService,
  ) {
  }

  changeAliasName(receiver: string, newName: string) {
    this.receiverAlias.setAlias(receiver, newName)
  }

}
